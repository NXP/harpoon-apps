/*
 * Copyright 2019, 2021, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "controller.h"
#include "log.h"
#include "motor_control_api.h"
#include "io_device.h"
#if BUILD_MOTOR_IO_DEVICE == 1
#include "mcdrv.h"
#endif
#include "local_network.h"
#include "stats_task.h"
#include "user_button.h"

#define STRATEGY_CHANGE_DELAY      5000
#define CONTROLLER_STAT_PERIOD_SEC 2
#define MONITORING_STAT_PERIOD_MS  1000

char *state_names[] = {"CONTROL", "IO_DEVICE_MISSING", "STANDBY"};

static void controller_stats_print(void *data)
{
    struct controller_stats *stats_snap = data;

    INF("current state           : %s\n", state_names[stats_snap->state]);
    INF("state control           : %u\n", stats_snap->sm_control);
    INF("state io_device missing : %u\n", stats_snap->sm_io_device_missing);
    INF("state standby           : %u\n", stats_snap->sm_standby);
    INF("errors msg id: %u, src id: %u, motor id: %u, empty data: %u\n",
        stats_snap->err_msg_id, stats_snap->err_src_id,
        stats_snap->err_motor_id, stats_snap->err_invalid_len_received);
    INF("errors strat loop: %u, strat next: %u\n",
        stats_snap->err_strat_loop, stats_snap->err_strat_next);

    stats_snap->pending = false;
}

static void controller_stats_dump(struct controller_ctx *ctx)
{
    if (ctx->stats_snap.pending)
        return;

    ctx->stats.state = ctx->state;
    memcpy(&ctx->stats_snap, &ctx->stats, sizeof(struct controller_stats));
    ctx->stats_snap.pending = true;

    // Print controller data
    if (STATS_Async(controller_stats_print, &ctx->stats_snap) != pdTRUE)
        ctx->stats_snap.pending = false;
}

static inline struct controlled_io_device *dev_id_to_io_device_ctx(struct controller_ctx *ctx, int io_device_id)
{
    unsigned int i;
    struct controlled_io_device *io_device = NULL;

    for (i = 0; i < ctx->num_io_device; i++)
        if (io_device_id == ctx->io_devices[i].id)
            io_device = &ctx->io_devices[i];

    return io_device;
}

static inline struct controlled_motor_ctx *motor_id_to_motor_data(struct controlled_io_device *io_device, int motor_id)
{
    if (motor_id < io_device->num_motors)
        return io_device->motors[motor_id];
    else
        return NULL;
}

static void __controller_monitoring_send(void *data)
{
    struct controller_ctx *ctx = data;

    monitoring_stats_send(ctx->monitoring_stats_ctx, &ctx->msg);
    ctx->msg_pending = false;
}

static void controller_monitoring_send(struct controller_ctx *ctx)
{
    if (ctx->msg_pending)
        return;

    ctx->msg.id = ctx->c_task->id;
    ctx->msg.state = ctx->state;
    ctx->msg.msg_type = MSG_CTRLER;

    control_strategy_get_motor_monitoring(ctx->strategy, ctx->msg.motor_stats, MONITOR_MAX_MOTOR);
    cyclic_task_get_monitoring(ctx->c_task, &ctx->msg.cyclic_task_stats, MONITOR_MAX_SOCKET);
    ctx->msg_pending = true;

    if (STATS_Async(__controller_monitoring_send, ctx) != pdTRUE)
        ctx->msg_pending = false;
}

static void controller_control_err_cb(void *data, int err)
{
    struct controller_ctx *ctx = data;

    if (err < 0)
        ctx->control_error = true;
}

static bool check_all_io_devices_connected(struct controller_ctx *ctx)
{
    unsigned int i = 0;
    uint16_t num_io_device_connected = 0;

    for (i = 0; i < ctx->num_io_device; i++) {
        if (ctx->io_devices[i].connected)
            num_io_device_connected++;
    }

    if (num_io_device_connected == ctx->num_io_device)
        return true;
    else
        return false;
}

static bool button_press_event_check(struct controller_ctx *ctx)
{
    enum event evt;
    bool ret = false;

    // Handle events coming from user button
    while (xQueueReceive(ctx->event_queue, &evt, 0) == pdTRUE) {
        if (evt == BUTTON_PRESSED)
            ret = true;
    }

    return ret;
}

static void controller_set_state(struct controller_ctx *ctx, sm_controller_state_t new_state)
{
    if (new_state != ctx->state) {
        ctx->state = new_state;
    }
}

static void controller_send(struct controller_ctx *ctx)
{
    unsigned int i, j;

    struct msg_set_iq msg_to_send;
    msg_to_send.num_msg = ctx->num_io_device;

    if (ctx->state == IO_DEVICE_MISSING)
        msg_to_send.action = HOLD;
    else
        msg_to_send.action = APPLY_IQ;

    // Fill structure with io_device id and corresponding data
    for (i = 0; i < ctx->num_io_device; i++) {
        for (j = 0; j < ctx->io_devices[i].num_motors; j++) {
            msg_to_send.msg_array[i].io_device_id = ctx->io_devices[i].id;
            msg_to_send.msg_array[i].motor_id = j;
            msg_to_send.msg_array[i].iq_req = control_strategy_get_iq(ctx->io_devices[i].motors[j]);
        }
    }

    if (ctx->motor_local)
        local_controller_transmit(ctx, &msg_to_send);
    else
        cyclic_net_transmit(ctx->c_task, MSG_SET_IQ, &msg_to_send, sizeof(msg_to_send));
}

static void controller_state_control(struct controller_ctx *ctx)
{
    int cmd_client_state;

    // Handle events coming from user button
    if (button_press_event_check(ctx) && !ctx->stopped) {
        control_strategy_next_strategy(ctx->strategy);
    }

    if (!check_all_io_devices_connected(ctx)) {
        controller_set_state(ctx, IO_DEVICE_MISSING);
        ctx->restart_delay = (RESTART_DELAY_MS * NSECS_PER_MSEC) /
                             ctx->c_task->params.task_period_ns;
        return;
    }

    // Handle events coming from command client
    if (ctx->cmd_client_ctx) {
        cmd_client_state = command_client_get_state(ctx->cmd_client_ctx);
        if (cmd_client_state == CMD_STATE_STOP && !ctx->stopped) {
            ctx->stopped = true;
            control_strategy_set_strategy(ctx->strategy, STOP);
        } else if (cmd_client_state == CMD_STATE_GO && ctx->stopped) {
            ctx->stopped = false;
            control_strategy_previous_strategy(ctx->strategy);
        }
    }

    control_strategy_state_machine(ctx->strategy);

    if (ctx->control_error) {
        ctx->stats.err_strat_loop++;
        ctx->control_error = false;
        controller_set_state(ctx, STANDBY);
    }
}

static void controller_state_io_device_missing(struct controller_ctx *ctx)
{
    if (button_press_event_check(ctx) && !ctx->stopped)
        control_strategy_next_strategy(ctx->strategy);

    if (check_all_io_devices_connected(ctx)) {
        if (!ctx->restart_delay--) {
            control_strategy_reset_strategy(ctx->strategy);
            controller_set_state(ctx, CONTROL);
        }
    } else {
        ctx->restart_delay = (RESTART_DELAY_MS * NSECS_PER_MSEC) /
                             ctx->c_task->params.task_period_ns;
    }
}

static void controller_state_standby(struct controller_ctx *ctx)
{
    if (button_press_event_check(ctx)) {
        control_strategy_reset_strategy(ctx->strategy);
        controller_set_state(ctx, CONTROL);
    }
}

static void run_controller_state(struct controller_ctx *ctx)
{
    switch (ctx->state) {
    case CONTROL:
        controller_state_control(ctx);
        ctx->stats.sm_control++;
        break;
    case IO_DEVICE_MISSING:
        controller_state_io_device_missing(ctx);
        ctx->stats.sm_io_device_missing++;
        break;
    case STANDBY:
        controller_state_standby(ctx);
        ctx->stats.sm_standby++;
        break;
    default:
        break;
    }
}

static void controller_loop(void *data, int timer_status)
{
    unsigned int i = 0;
    struct controller_ctx *ctx = data;
    unsigned int num_sched_stats = CONTROLLER_STAT_PERIOD_SEC *
                                   (NSECS_PER_SEC_F / ctx->c_task->task->params->task_period_ns);
    unsigned int num_sched_monitoring = MONITORING_STAT_PERIOD_MS *
                                        (NSECS_PER_MSEC / ctx->c_task->task->params->task_period_ns);

    // Reset Iq to 0 in case motor ends-up not being driven
    control_strategy_reset_iq(ctx->strategy);

    run_controller_state(ctx);

    // Send computed data to the io_devices
    controller_send(ctx);

    // Mark all io_devices data as handled
    for (i = 0; i < ctx->num_io_device; i++) {
        ctx->io_devices[i].connected = 0;
    }

    // Monitoring stats
    if (ctx->c_task->task->stats.sched % num_sched_monitoring == 0) {
        controller_monitoring_send(ctx);
    }

    // Stats printing
    if (ctx->c_task->task->stats.sched % num_sched_stats == 0) {
        controller_stats_dump(ctx);
        control_strategy_stats_dump(ctx->strategy);
    }
}

void controller_net_receive(void *data, int msg_id, int src_id, void *buf, int len)
{
    unsigned int i;
    struct controller_ctx *ctx = data;
    struct msg_feedback *msg_recv = buf;
    struct controlled_io_device *io_device;
    struct controlled_motor_ctx *motor;

    if (msg_id == MSG_FEEDBACK) {
        // Check size of data received
        if (len != sizeof(struct msg_feedback)) {
            ctx->stats.err_invalid_len_received++;
            return;
        }

        io_device = dev_id_to_io_device_ctx(ctx, src_id);
        if (!io_device) {
            ctx->stats.err_src_id++;
            return;
        }

        // If no errors occured on the io_device, it is considered connected
        if (!msg_recv->status)
            io_device->connected = 1;

        for (i = 0; i < msg_recv->num_msg; i++) {
            // Get context of io_device related to motor_id
            motor = motor_id_to_motor_data(io_device, msg_recv->msg_array[i].motor_id);
            if (!motor) {
                ctx->stats.err_motor_id++;
                continue;
            }

            // Update data of corresponding controlled io_device
            control_strategy_set_feedback(motor, &msg_recv->msg_array[i]);
        }
    } else
        ctx->stats.err_msg_id++;
}

int controller_init(struct controller_ctx *ctx, struct cyclic_task *c_task, bool motor_local,
                    control_strategies_t first_strategy, bool cmd_client)
{
    unsigned int i, j;
    uint64_t now;

    ctx->motor_local = motor_local;

    if (ctx->motor_local) {
        ctx->num_io_device = 1;
    } else {
        ctx->num_io_device = c_task->num_peers;
    }

    /* Initialize queue that handles button events */
    ctx->event_queue = xQueueCreate(1, sizeof(enum event));
    if (!ctx->event_queue) {
        ERR("Unable to create queue\n");
        goto err;
    }

    if (user_button_add_event_queue(&ctx->event_queue) < 0) {
        ERR("Unable to add event queue for user button events\n");
        goto err_del_queue;
    }

    ctx->state = CONTROL;
    ctx->strategy_change_counter = 0;
    ctx->last_control_strategy = first_strategy;
    ctx->stopped = false;
    ctx->control_error = false;
    ctx->cmd_client_ctx = NULL;

    // Initialize control strategy
    if (control_strategy_context_init(&ctx->strategy, first_strategy, c_task->params.task_period_ns) < 0) {
        ERR("Unable to initialize control strategy\n");
        goto err_del_queue;
    }

    // Init io_devices
    for (i = 0; i < ctx->num_io_device; i++) {
        ctx->io_devices[i].id = c_task->rx_socket[i].peer_id;
        ctx->io_devices[i].connected = 0;
        ctx->io_devices[i].num_motors = 1;
        for (j = 0; j < ctx->io_devices[i].num_motors; j++) {
            genavb_clock_gettime64(c_task->params.clk_id, &now);
            ctx->io_devices[i].motors[j] = control_strategy_register_motor(ctx->strategy, ctx->io_devices[i].id, j, now);
        }
    }

    // Register control strategy error callback
    control_strategy_set_error_callback(ctx->strategy, controller_control_err_cb, ctx);

    if (cmd_client) {
        if (command_client_start(&ctx->cmd_client_ctx) < 0) {
            ERR("Unable to start command client\n");
        }
    }

    if (monitoring_stats_open(&ctx->monitoring_stats_ctx) < 0) {
        ERR("Failed to open monitoring stats socket\n");
        goto err_del_queue;
    }

    ctx->c_task = c_task;
    if (cyclic_task_init(c_task, controller_net_receive, controller_loop, ctx) < 0)
        goto err_del_queue;

    INF("Controller Init Successful\n");

    return 0;

err_del_queue:
    vQueueDelete(ctx->event_queue);
err:
    return -1;
}
