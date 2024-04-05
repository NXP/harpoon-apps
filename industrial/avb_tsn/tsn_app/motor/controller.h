/*
 * Copyright 2019, 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "motor_control.h"
#include "scenarios.h"
#include "traj_planner.h"
#include "cyclic_task.h"
#include "current_control.h"
#include "queue.h"
#include "control_strategies.h"
#include "monitoring_stats.h"
#include "command_client.h"

#define RESTART_DELAY_MS (2000)

typedef enum {
    CONTROL,
    IO_DEVICE_MISSING,
    STANDBY,
} sm_controller_state_t;

struct controller_stats {
    bool pending;
    sm_controller_state_t state;
    float diff_pos_real_io_devices;
    float diff_pos_target_io_devices;
    uint32_t err_msg_id;
    uint32_t err_invalid_len_received;
    uint32_t err_src_id;
    uint32_t err_motor_id;
    uint32_t err_strat_loop;
    uint32_t err_strat_next;
    uint32_t sm_control;
    uint32_t sm_io_device_missing;
    uint32_t sm_standby;
};

struct controlled_io_device {
    int id;
    int num_motors;
    int connected;
    struct controlled_motor_ctx *motors[NB_MOTORS_MAX];
};

struct controller_ctx {
    uint16_t num_io_device;
    sm_controller_state_t state;
    struct controlled_io_device io_devices[NB_IO_DEVICE_MAX];
    struct control_strategy_ctx *strategy;
    uint32_t strategy_change_counter;
    struct cyclic_task *c_task;
    bool motor_local;
    struct controller_stats stats;
    struct controller_stats stats_snap;
    QueueHandle_t event_queue;
    int restart_delay;
    struct monitoring_stats_ctx *monitoring_stats_ctx;
    struct monitoring_msg msg;
    bool msg_pending;
    struct command_client_ctx *cmd_client_ctx;
    bool stopped;
    bool control_error;
    control_strategies_t last_control_strategy;
};

int controller_init(struct controller_ctx *ctx, struct cyclic_task *c_task, bool motor_local,
                    control_strategies_t first_strategy, bool cmd_client);
int controller_exit(struct controller_ctx *ctx);
void controller_net_receive(void *data, int msg_id, int src_id, void *buf, int len);

#endif /* _CONTROLLER_H_ */
