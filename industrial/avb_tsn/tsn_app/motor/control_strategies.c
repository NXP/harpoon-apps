/*
 * Copyright 2019, 2021, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "control_strategies.h"
#include "log.h"
#include "slist.h"
#include "types.h"
#include "stats_task.h"
#include "network_stats.h"
#include "math.h"
#include "motor_params.h"
#include "stats.h"
#include "traj_planner.h"

/* ----- Definitions ----- */
#define ZERO_ALIGNEMENT_DELAY 5000
#define MARGIN_POSITION_ERROR 0.055
#define POSITION_ACCURACY     0.02
#define NOTCH_POSITIONS       45.0 // in degrees
#define NOTCH_ACCURACY        7.0
#define DEG_SCALE_STATS       100.0
#define DEG_SCALE_HIST        10.0
#define STOP_DELAY            5000

#define IDENTIFICATION_IQ_MIN                0.25
#define IDENTIFICATION_IQ_MAX                0.5
#define IDENTIFICATION_IQ_STEP               0.05
#define IDENTIFICATION_STATE_MS              5000ULL /* ms */
#define IDENTIFICATION_MAX_MOTOR             2
#define IDENTIFICATION_MAX_STEPS             6

char *strategy_names[] = {"SYNCHRONIZED", "FOLLOW", "HOLD_INDEX", "INTERLACED", "STOP", "IDENTIFY"};
char *strategy_states_names[] = {"PREPARE", "STARTUP", "INIT", "STRATEGY", "RESET"};

typedef enum control_strategy_state {
    PREPARE,
    STARTUP,
    INIT,
    CONTROL,
    RESET
} control_strategy_state_t;

typedef enum {
    CTRL_STRAT_ERROR = -1,
    CTRL_STRAT_WAIT  = 0,
    CTRL_STRAT_OK    = 1
} ctrl_strategy_return_codes_t;

struct control_strategy_ops {
    ctrl_strategy_return_codes_t (*prepare)(struct control_strategy_ctx *ctx);
    ctrl_strategy_return_codes_t (*startup)(struct control_strategy_ctx *ctx);
    ctrl_strategy_return_codes_t (*init)(struct control_strategy_ctx *ctx);
    ctrl_strategy_return_codes_t (*loop)(struct control_strategy_ctx *ctx);
    ctrl_strategy_return_codes_t (*reset)(struct control_strategy_ctx *ctx);
};

struct stats_control_strategy {
    bool pending;
    control_strategies_t current_strategy;
    control_strategies_t old_strategy;
    control_strategy_state_t state;
};

struct stats_controlled_motor {
    bool pending;
    float pos_target;
    float startup_offset;
    float pos_real;
    float speed_real;
    float iq_req;
    uint32_t err_margin;
    uint32_t err_margin_stop;
    float last_margin_error;
    struct stats pos_err_deg;
    struct hist pos_err_deg_hist;
    int32_t pos_err_max;
};

struct interlaced_ctx {
    struct controlled_motor_ctx *motor_static;
    struct controlled_motor_ctx *motor_dynamic;
    uint32_t startup_delay;
};

struct synchro_ctx {
};

struct identification_ctx {
    struct interlaced_ctx interlaced;

    unsigned int state;

    uint32_t start_count;
    uint32_t state_count;

    float iq_min;
    float iq_step;
    unsigned int i;
    unsigned int i_max;
    unsigned int motor;

    float t0;
    float t1;
    float t2;

    float speed_t0;
    float speed_t1;
    float speed_t2;

    float pos_t0;
    float pos_t1;
    float pos_t2;

    float speed_final[IDENTIFICATION_MAX_MOTOR][IDENTIFICATION_MAX_STEPS];

    float alpha;
    float b_J[IDENTIFICATION_MAX_MOTOR][IDENTIFICATION_MAX_STEPS];
    uint16_t id[IDENTIFICATION_MAX_MOTOR];
};

struct controlled_motor_ctx {
    struct slist_node node;
    uint16_t id;
    float startup_offset;
    float iq_req;
    struct motor_feedback fb;
    float pos_target;
    float speed_target;
    float accel_target;
    float speed_max;
    float accel_max;
    struct traj_trapez traj;
    struct scenario_ctx scenario;
    struct iq_control runtime_iq_ctrl;
    struct iq_control startup_iq_ctrl;
    struct motor_control_params params;
    uint32_t demo_count;
    uint64_t absolute_time_beginning;
    uint32_t seqid_stats;
    struct stats_controlled_motor stats_snap;
    struct stats_controlled_motor stats;
    uint32_t nb_errors;
    struct net_stat_msg net_stats;
};

struct control_strategy_ctx {
    struct slist_head motor_list;
    control_strategies_t current_strategy;
    control_strategies_t old_strategy;
    uint16_t num_strategies;
    uint16_t num_motors;
    struct control_strategy_ops ops[MAX_NUM_CONTROL_STRATEGIES];
    struct network_stats_ctx *net_stats_ctx;
    bool net_stats_pending;
    control_strategy_state_t state;
    struct stats_control_strategy stats_snap;
    struct stats_control_strategy stats;
    void (*err_cb)(void *, int);
    void *err_cb_user_data;
    union {
        struct interlaced_ctx interlaced_context;
        struct synchro_ctx synchro;
        struct identification_ctx identification;
    };
    unsigned int app_period_ns;
};

static struct control_strategy_ctx control_strategy;
static struct control_strategy_ctx *control_strategy_h = NULL;

static bool check_margin_position(struct controlled_motor_ctx *motor, float pos_real, float pos_target, float margin)
{
    float gap;

    gap = fabs(pos_real - pos_target);
    if (gap > margin) {
        motor->stats.err_margin++;
        motor->stats.last_margin_error = gap * 360.0;
        return false;
    } else {
        return true;
    }
}

static void update_pos_err_stat(struct controlled_motor_ctx *motor, float pos, float pos_target)
{
    int pos_err_deg_stats;
    unsigned int pos_err_deg_hist;

    pos_err_deg_stats = (fabs(pos - pos_target)) * 360.0 * DEG_SCALE_STATS;
    pos_err_deg_hist = (fabs(pos - pos_target)) * 360.0 * DEG_SCALE_HIST;

    stats_update(&motor->stats.pos_err_deg, pos_err_deg_stats);
    hist_update(&motor->stats.pos_err_deg_hist, pos_err_deg_hist);

    if (pos_err_deg_stats > abs(motor->stats.pos_err_max)) {
        motor->stats.pos_err_max = (pos - pos_target) * 360.0 * DEG_SCALE_STATS;
    }
}

static void switch_motor_alternation_interlaced(struct control_strategy_ctx *ctx)
{
    struct controlled_motor_ctx *motor_temp = NULL;

    motor_temp = ctx->interlaced_context.motor_dynamic;
    ctx->interlaced_context.motor_dynamic = ctx->interlaced_context.motor_static;
    ctx->interlaced_context.motor_static = motor_temp;
}

static ctrl_strategy_return_codes_t strategy_generic_prepare(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor = NULL;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->startup_offset = (int)motor->fb.pos;

        iq_controller_init(&motor->startup_iq_ctrl, CTRL_MODE_TRAJECTORY,
                           &motor->params);
    }

    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_generic_startup(struct control_strategy_ctx *ctx)
{
    uint16_t nb_io_device_at_target = 0;
    struct slist_node *entry;
    struct controlled_motor_ctx *motor = NULL;
    uint16_t i = 0;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        compute_trap_traj(&motor->traj,
                          motor->demo_count,
                          motor->startup_offset,
                          motor->fb.pos,
                          motor->fb.speed,
                          30.0,
                          ctx->app_period_ns,
                          &motor->params);

        // Compute number of cycles to reach zero
        uint32_t cycle_to_reach =
            motor->traj.start_cycle + motor->traj.cycles_total;

        // Compute how many motors are aligned
        if (motor->demo_count > cycle_to_reach + ZERO_ALIGNEMENT_DELAY) {
            nb_io_device_at_target++;
        }

        /* Evaluate feedforward values at time t */
        evaluate_trap_traj(&motor->traj,
                           &motor->pos_target,
                           &motor->speed_target,
                           &motor->accel_target,
                           motor->demo_count);

        /* Compute Iq required for this io_device */
        motor->iq_req = get_iq_command(&motor->startup_iq_ctrl,
                                       motor->fb.pos,
                                       motor->fb.speed,
                                       motor->pos_target,
                                       motor->speed_target,
                                       motor->accel_target);

        // Increase cycle count
        motor->demo_count++;

        i++;
    }

    // Go to control mode and select strategy
    if (nb_io_device_at_target == ctx->num_motors) {
        return CTRL_STRAT_OK;
    }

    return CTRL_STRAT_WAIT;
}

/* -----  SYNCHRONIZED STRATEGY  ----- */

static ctrl_strategy_return_codes_t strategy_synchronized_init(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;
    uint8_t i = 0;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);
        if (scenario_init(&motor->scenario, SCENARIO_SYNCHRONIZED, ctx->app_period_ns) < 0) {
            ERR("Error when initializing scenario %u\n", 0);
            goto err;
        }

        // Initialize Iq controller of each io_device
        iq_controller_init(&motor->runtime_iq_ctrl, CTRL_MODE_TRAJECTORY,
                           &motor->params);

        reset_trap_traj(&motor->traj);
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;

        i++;
    }

    return CTRL_STRAT_OK;

err:
    return CTRL_STRAT_ERROR;
}

static ctrl_strategy_return_codes_t strategy_synchronized_loop(struct control_strategy_ctx *ctx)
{
    uint16_t nb_io_device_at_target = 0;
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    /* Check position error before applying control */
    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        update_pos_err_stat(motor, motor->fb.pos - motor->startup_offset, motor->pos_target);

        if (!check_margin_position(motor, (motor->fb.pos - motor->startup_offset), motor->pos_target, MARGIN_POSITION_ERROR)) {
            motor->nb_errors++;
            if (motor->nb_errors > 3) {
                motor->stats.err_margin_stop++;
                goto err;
            }
        } else {
            if (motor->nb_errors > 0)
                motor->nb_errors--;
        }
    }

    /* Apply control for each io_device */
    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);
        // Get scenario data
        get_scenario_block(&motor->scenario);

        // Correct offsets
        motor->fb.pos -= motor->startup_offset;

        /* Make a new computation only if pos_target is different */
        compute_trap_traj(&motor->traj,
                          motor->demo_count,
                          motor->scenario.pos_target,
                          motor->fb.pos,
                          motor->fb.speed,
                          motor->scenario.speed_max,
                          ctx->app_period_ns,
                          &motor->params);

        // Compute number of cycles of the current scenario
        uint32_t cycle_to_reach =
            motor->traj.start_cycle + motor->traj.cycles_total;

        // Compute how many io_devices ended their scenario
        if (motor->demo_count > cycle_to_reach + motor->scenario.after_delay) {
            nb_io_device_at_target++;
        }

        /* Evaluate feedforward values at time t */
        evaluate_trap_traj(&motor->traj,
                           &motor->pos_target,
                           &motor->speed_target,
                           &motor->accel_target,
                           motor->demo_count);

        /* Compute Iq required for this io_device */
        motor->iq_req = get_iq_command(&motor->runtime_iq_ctrl,
                                       motor->fb.pos,
                                       motor->fb.speed,
                                       motor->pos_target,
                                       motor->speed_target,
                                       motor->accel_target);

        // Increase cycle count
        motor->demo_count++;
    }

    // Go to next scenario for every io_devices at the same time
    if (nb_io_device_at_target == ctx->num_motors) {
        slist_for_each (&ctx->motor_list, entry) {
            motor = container_of(entry, struct controlled_motor_ctx, node);
            next_block_scenario(&motor->scenario);
        }
    }

    return CTRL_STRAT_OK;

err:
    return CTRL_STRAT_ERROR;
}

static ctrl_strategy_return_codes_t strategy_synchronized_reset(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->iq_req = 0.0;
        reset_trap_traj(&motor->traj);
        reset_scenario(&motor->scenario);
        motor->startup_offset = 0.0;
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
        motor->speed_max = 0.0;
    }

    return CTRL_STRAT_OK;
}

/* -----  FOLLOW STRATEGY  ----- */

static ctrl_strategy_return_codes_t strategy_follow_init(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    if (ctx->num_motors < 2) {
        ERR("Unable to initialize FOLLOW mode because less than two io_devices are controlled\n");
        goto err;
    }

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);
        // Initialize Iq controller of each io_device
        iq_controller_init(&motor->runtime_iq_ctrl, CTRL_MODE_POSITION,
                           &motor->params);
        reset_trap_traj(&motor->traj);
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
    }

    return CTRL_STRAT_OK;

err:
    return CTRL_STRAT_ERROR;
}

static ctrl_strategy_return_codes_t strategy_follow_loop(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;
    struct controlled_motor_ctx *motor_master = NULL;

    // Correct offsets
    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->fb.pos -= motor->startup_offset;
        if (motor->id == 256) {
            motor_master = motor;
        }
    }

    if (motor_master == NULL) {
        ERR("Motor master not found\n");
        goto err;
    }

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        if (motor == motor_master) {
            motor->iq_req = 0.0;
        } else {
            if (!check_margin_position(motor, motor->fb.pos, motor_master->fb.pos, MARGIN_POSITION_ERROR)) {
                goto err;
            }

            motor->pos_target = motor_master->fb.pos;

            motor->iq_req = get_iq_command(&motor->runtime_iq_ctrl,
                                           motor->fb.pos,
                                           motor->fb.speed,
                                           motor->pos_target,
                                           0.0,
                                           0.0);
        }
    }

    return CTRL_STRAT_OK;

err:
    return CTRL_STRAT_ERROR;
}

static ctrl_strategy_return_codes_t strategy_follow_reset(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->iq_req = 0.0;

        reset_trap_traj(&motor->traj);
        reset_scenario(&motor->scenario);
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
    }
    return CTRL_STRAT_OK;
}

/* -----  HOLD_INDEX STRATEGY  ----- */

static ctrl_strategy_return_codes_t strategy_hold_index_init(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);
        // Initialize Iq controller of each io_device
        iq_controller_init(&motor->runtime_iq_ctrl, CTRL_MODE_POSITION,
                           &motor->params);
        reset_trap_traj(&motor->traj);
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
    }

    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_hold_index_loop(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    /* Apply control for each io_device */
    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        // Correct offsets
        motor->fb.pos -= motor->startup_offset;

        motor->pos_target = 0.0;

        /* Compute Iq required for this io_device */
        motor->iq_req = get_iq_command(&motor->runtime_iq_ctrl,
                                       motor->fb.pos,
                                       motor->fb.speed,
                                       motor->pos_target,
                                       0.0,
                                       0.0);
    }

    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_hold_index_reset(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->iq_req = 0.0;
        reset_trap_traj(&motor->traj);
        reset_scenario(&motor->scenario);
        motor->startup_offset = 0.0;
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
        motor->speed_max = 0.0;
    }

    return CTRL_STRAT_OK;
}

/* -----  INTERLACED STRATEGY  ----- */

static ctrl_strategy_return_codes_t strategy_interlaced_prepare(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;
    uint16_t i = 0;
    struct controlled_motor_ctx *motor_static = NULL;
    struct controlled_motor_ctx *motor_dynamic = NULL;
    float delta_notch_deg;
    ctrl_strategy_return_codes_t rc = CTRL_STRAT_OK;

    if (ctx->num_motors != 2) {
        ERR("Unable to initialize INTERLACED mode because less than two io_devices are controlled\n");
        rc = CTRL_STRAT_ERROR;
        goto exit;
    }

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        if (i == 0 && ctx->interlaced_context.motor_dynamic == NULL) {
            ctx->interlaced_context.motor_dynamic = motor;
        } else if (i == 1 && ctx->interlaced_context.motor_static == NULL) {
            ctx->interlaced_context.motor_static = motor;
        }

        i++;
    }

    motor_static = ctx->interlaced_context.motor_static;
    motor_dynamic = ctx->interlaced_context.motor_dynamic;

    // Startup offset is in integer revolutions
    motor_static->startup_offset = (int)motor_static->fb.pos;
    motor_dynamic->startup_offset = (int)motor_dynamic->fb.pos;

    // Init controller types
    iq_controller_init(&motor_static->startup_iq_ctrl, CTRL_MODE_POSITION,
                       &motor_static->params);
    iq_controller_init(&motor_dynamic->startup_iq_ctrl, CTRL_MODE_TRAJECTORY,
                       &motor_dynamic->params);

    // Check if the dynamic motor is physically able to rotate (position is modulo 45 deg)
    delta_notch_deg = (motor_static->fb.pos - (int)motor_static->fb.pos) * 360;
    delta_notch_deg -= ((int)delta_notch_deg / (int)NOTCH_POSITIONS) * NOTCH_POSITIONS;

    if (delta_notch_deg > (NOTCH_POSITIONS / 2))
        delta_notch_deg -= NOTCH_POSITIONS;

    if (abs(delta_notch_deg) <= NOTCH_ACCURACY) {
        motor_static->pos_target = motor_static->fb.pos - ((float)delta_notch_deg / 360.0);

        // We set the startup delay used to wait for static motor to hold the position
        ctx->interlaced_context.startup_delay = motor_static->demo_count + ZERO_ALIGNEMENT_DELAY;
    } else {
        DBG("Dynamic motor can't rotate, try other motor, (delta_notch_deg = %f)\n", delta_notch_deg);
        switch_motor_alternation_interlaced(ctx);
        rc = CTRL_STRAT_WAIT;
        goto exit;
    }

exit:
    return rc;
}

static ctrl_strategy_return_codes_t strategy_interlaced_startup(struct control_strategy_ctx *ctx)
{
    uint32_t cycle_to_reach;
    struct controlled_motor_ctx *motor_static = ctx->interlaced_context.motor_static;
    struct controlled_motor_ctx *motor_dynamic = ctx->interlaced_context.motor_dynamic;
    ctrl_strategy_return_codes_t rc = CTRL_STRAT_WAIT;

    // Static motor keeps the notch position
    motor_static->iq_req = get_iq_command(&motor_static->startup_iq_ctrl,
                                          motor_static->fb.pos,
                                          motor_static->fb.speed,
                                          motor_static->pos_target,
                                          0.0,
                                          0.0);

    motor_static->demo_count++;

    // If the static motor is at the notch and the delay has passed, we start aligning the dynamic motor
    if (check_margin_position(motor_static, motor_static->fb.pos, motor_static->pos_target, POSITION_ACCURACY) &&
        motor_static->demo_count > ctx->interlaced_context.startup_delay) {
        compute_trap_traj(&motor_dynamic->traj,
                          motor_dynamic->demo_count,
                          motor_dynamic->startup_offset,
                          motor_dynamic->fb.pos,
                          motor_dynamic->fb.speed,
                          30.0,
                          ctx->app_period_ns,
                          &motor_dynamic->params);

        /* Evaluate feedforward values at time t */
        evaluate_trap_traj(&motor_dynamic->traj,
                           &motor_dynamic->pos_target,
                           &motor_dynamic->speed_target,
                           &motor_dynamic->accel_target,
                           motor_dynamic->demo_count);

        /* Compute Iq required for the dynamic motor */
        motor_dynamic->iq_req = get_iq_command(&motor_dynamic->startup_iq_ctrl,
                                               motor_dynamic->fb.pos,
                                               motor_dynamic->fb.speed,
                                               motor_dynamic->pos_target,
                                               motor_dynamic->speed_target,
                                               motor_dynamic->accel_target);

        // Increase cycle count
        motor_dynamic->demo_count++;

        // Compute number of cycles to reach the target
        cycle_to_reach =
            motor_dynamic->traj.start_cycle + motor_dynamic->traj.cycles_total;

        // Compute how many motors are aligned at the relative zero (startup offset)
        if (motor_dynamic->demo_count > cycle_to_reach + ZERO_ALIGNEMENT_DELAY) {
            // Here the dynamic motor has finished its alignment to the relative zero
            // We check if the static motor is at the startup offset, if so, both motors are aligned
            // otherwise, we need to align the static motor and to hold the one that has finished to align
            if (check_margin_position(motor_static, motor_static->fb.pos, motor_static->startup_offset, POSITION_ACCURACY)) {
                // All motors are aligned, we leave the startup state and reset the boolean for the next entrance in startup state
                rc = CTRL_STRAT_OK;
            } else {
                // Static motor is at a notch but not the one corresponding to the relative zero, so we switch motor alternation
                // in order to align the static motor
                switch_motor_alternation_interlaced(ctx);
                motor_static = ctx->interlaced_context.motor_static;
                motor_dynamic = ctx->interlaced_context.motor_dynamic;

                motor_static->pos_target = motor_static->startup_offset;
                ctx->interlaced_context.startup_delay = motor_static->demo_count + ZERO_ALIGNEMENT_DELAY;
                // Init controller types
                iq_controller_init(&motor_static->startup_iq_ctrl, CTRL_MODE_POSITION,
                                   &motor_static->params);
                iq_controller_init(&motor_dynamic->startup_iq_ctrl, CTRL_MODE_TRAJECTORY,
                                   &motor_dynamic->params);
            }
        }
    }

    return rc;
}

static ctrl_strategy_return_codes_t strategy_interlaced_init(struct control_strategy_ctx *ctx)
{
    struct controlled_motor_ctx *motor_static = ctx->interlaced_context.motor_static;
    struct controlled_motor_ctx *motor_dynamic = ctx->interlaced_context.motor_dynamic;

    if (scenario_init(&motor_static->scenario, SCENARIO_INTERLACED, ctx->app_period_ns) < 0) {
        ERR("Error when initializing scenario %u\n", 0);
        goto err;
    }

    if (scenario_init(&motor_dynamic->scenario, SCENARIO_INTERLACED, ctx->app_period_ns) < 0) {
        ERR("Error when initializing scenario %u\n", 0);
        goto err;
    }

    iq_controller_init(&motor_dynamic->runtime_iq_ctrl, CTRL_MODE_TRAJECTORY,
                       &motor_dynamic->params);

    iq_controller_init(&motor_static->runtime_iq_ctrl, CTRL_MODE_POSITION,
                       &motor_static->params);

    reset_trap_traj(&motor_dynamic->traj);

    motor_dynamic->pos_target = 0.0;
    motor_dynamic->speed_target = 0.0;
    motor_dynamic->accel_target = 0.0;
    motor_static->pos_target = 0.0;
    motor_static->speed_target = 0.0;
    motor_static->accel_target = 0.0;

    return CTRL_STRAT_OK;

err:
    return CTRL_STRAT_ERROR;
}

static ctrl_strategy_return_codes_t strategy_interlaced_loop(struct control_strategy_ctx *ctx)
{
    uint32_t cycle_to_reach;
    struct controlled_motor_ctx *motor_static = ctx->interlaced_context.motor_static;
    struct controlled_motor_ctx *motor_dynamic = ctx->interlaced_context.motor_dynamic;

    // Correct offsets
    motor_dynamic->fb.pos -= motor_dynamic->startup_offset;
    motor_static->fb.pos -= motor_static->startup_offset;

    update_pos_err_stat(motor_dynamic, motor_dynamic->fb.pos, motor_dynamic->pos_target);
    update_pos_err_stat(motor_static, motor_static->fb.pos, motor_static->pos_target);

    // Check if error isn't too big for both motors
    if (!check_margin_position(motor_dynamic, motor_dynamic->fb.pos, motor_dynamic->pos_target, MARGIN_POSITION_ERROR)) {
        motor_dynamic->nb_errors++;
        if (motor_dynamic->nb_errors > 3) {
            motor_dynamic->stats.err_margin_stop++;
            goto err;
        }
    } else {
        if (motor_dynamic->nb_errors > 0)
            motor_dynamic->nb_errors--;
    }

    if (!check_margin_position(motor_static, motor_static->fb.pos, motor_static->pos_target, MARGIN_POSITION_ERROR)) {
        motor_static->nb_errors++;
        if (motor_static->nb_errors > 3) {
            motor_static->stats.err_margin_stop++;
            goto err;
        }
    } else {
        if (motor_static->nb_errors > 0)
            motor_static->nb_errors--;
    }

    // Dynamic motor will follow scenario steps
    get_scenario_block(&motor_dynamic->scenario);

    compute_trap_traj(&motor_dynamic->traj,
                      motor_dynamic->demo_count,
                      motor_dynamic->scenario.pos_target,
                      motor_dynamic->fb.pos,
                      motor_dynamic->fb.speed,
                      motor_dynamic->scenario.speed_max,
                      ctx->app_period_ns,
                      &motor_dynamic->params);

    evaluate_trap_traj(&motor_dynamic->traj,
                       &motor_dynamic->pos_target,
                       &motor_dynamic->speed_target,
                       &motor_dynamic->accel_target,
                       motor_dynamic->demo_count);

    ctx->interlaced_context.motor_dynamic->iq_req = get_iq_command(&motor_dynamic->runtime_iq_ctrl,
                                                                   motor_dynamic->fb.pos,
                                                                   motor_dynamic->fb.speed,
                                                                   motor_dynamic->pos_target,
                                                                   motor_dynamic->speed_target,
                                                                   motor_dynamic->accel_target);

    // Static motor keeps his last position
    ctx->interlaced_context.motor_static->iq_req = get_iq_command(&motor_static->runtime_iq_ctrl,
                                                                  motor_static->fb.pos,
                                                                  motor_static->fb.speed,
                                                                  motor_static->pos_target,
                                                                  0.0,
                                                                  0.0);

    // Compute number of cycles of the current scenario
    cycle_to_reach =
        motor_dynamic->traj.start_cycle + ctx->interlaced_context.motor_dynamic->traj.cycles_total;

    // Compute how many io_devices ended their scenario
    if ((motor_dynamic->demo_count > cycle_to_reach + motor_dynamic->scenario.after_delay) &&
        check_margin_position(motor_dynamic, motor_dynamic->fb.pos,
                              motor_dynamic->scenario.pos_target, POSITION_ACCURACY)) {
        next_block_scenario(&motor_dynamic->scenario);

        // Remember position to maintain static motor
        motor_dynamic->pos_target = motor_dynamic->scenario.pos_target;

        // Switch controller types
        iq_controller_init(&motor_dynamic->runtime_iq_ctrl, CTRL_MODE_POSITION,
                           &motor_dynamic->params);
        iq_controller_init(&motor_static->runtime_iq_ctrl, CTRL_MODE_TRAJECTORY,
                           &motor_static->params);

        switch_motor_alternation_interlaced(ctx);
    }

    motor_dynamic->demo_count++;

    return CTRL_STRAT_OK;

err:
    return CTRL_STRAT_ERROR;
}

static ctrl_strategy_return_codes_t strategy_interlaced_reset(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        ctx->interlaced_context.motor_dynamic = NULL;
        ctx->interlaced_context.motor_static = NULL;
        ctx->interlaced_context.startup_delay = 0;
        motor->iq_req = 0.0;
        reset_trap_traj(&motor->traj);
        reset_scenario(&motor->scenario);
        motor->startup_offset = 0.0;
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
        motor->speed_max = 0.0;
    }

    return CTRL_STRAT_OK;
}

/* -----  STOP STRATEGY  ----- */

static ctrl_strategy_return_codes_t strategy_stop_prepare(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor = NULL;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->startup_offset = (int)motor->fb.pos;

        iq_controller_init(&motor->startup_iq_ctrl, CTRL_MODE_POSITION,
                           &motor->params);

        motor->demo_count = 0;
    }

    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_stop_startup(struct control_strategy_ctx *ctx)
{
    uint16_t nb_io_device_at_target = 0;
    struct slist_node *entry;
    struct controlled_motor_ctx *motor = NULL;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        // Correct offsets
        motor->fb.pos -= motor->startup_offset;

        // Force motor to hold current position to stop the motors
        motor->pos_target = motor->fb.pos;
        motor->iq_req = get_iq_command(&motor->startup_iq_ctrl,
                                       motor->fb.pos,
                                       motor->fb.speed,
                                       motor->pos_target,
                                       0.0,
                                       0.0);

        // Wait enough time to reach zero speed
        if (motor->demo_count > STOP_DELAY) {
            nb_io_device_at_target++;
        }

        motor->demo_count++;
    }

    if (nb_io_device_at_target == ctx->num_motors) {
        return CTRL_STRAT_OK;
    }

    return CTRL_STRAT_WAIT;
}

static ctrl_strategy_return_codes_t strategy_stop_init(struct control_strategy_ctx *ctx)
{
    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_stop_loop(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    /* Apply control for each io_device */
    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        // Motor have been actively stopped during STARTUP process, now they are let free
        motor->iq_req = 0.0;
    }

    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_stop_reset(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->iq_req = 0.0;
        reset_trap_traj(&motor->traj);
        reset_scenario(&motor->scenario);
        motor->startup_offset = 0.0;
        motor->pos_target = 0.0;
        motor->speed_target = 0.0;
        motor->accel_target = 0.0;
        motor->speed_max = 0.0;
    }

    return CTRL_STRAT_OK;
}

/* -----  Identification STRATEGY  ----- */

#define ACCELERATING 0
#define COASTING 1
#define STOPPING 2
#define DONE 3

static ctrl_strategy_return_codes_t strategy_identification_init(struct control_strategy_ctx *ctx)
{
    struct identification_ctx *id = &ctx->identification;
    struct controlled_motor_ctx *motor_dynamic = id->interlaced.motor_dynamic;

    strategy_interlaced_init(ctx);

    id->state = ACCELERATING;
    id->state_count = (IDENTIFICATION_STATE_MS * NSECS_PER_MSEC) / ctx->app_period_ns;
    id->start_count = motor_dynamic->demo_count + 1;
    id->i = 0;
    id->motor = 0;
    id->iq_min = IDENTIFICATION_IQ_MIN;
    id->iq_step = IDENTIFICATION_IQ_STEP;

    id->i_max = (IDENTIFICATION_IQ_MAX - IDENTIFICATION_IQ_MIN) / IDENTIFICATION_IQ_STEP + 1;
    if (id->i_max > IDENTIFICATION_MAX_STEPS)
        id->i_max = IDENTIFICATION_MAX_STEPS;

    id->alpha = 0.2;

    motor_dynamic->demo_count++;

    return CTRL_STRAT_OK;
}

static ctrl_strategy_return_codes_t strategy_identification_loop(struct control_strategy_ctx *ctx)
{
    struct identification_ctx *id = &ctx->identification;
    struct controlled_motor_ctx *motor_static = id->interlaced.motor_static;
    struct controlled_motor_ctx *motor_dynamic = id->interlaced.motor_dynamic;
    int i, j;

    // Correct offsets
    motor_dynamic->fb.pos -= motor_dynamic->startup_offset;
    motor_static->fb.pos -= motor_static->startup_offset;

    switch (id->state) {
    case ACCELERATING:
        ctx->interlaced_context.motor_dynamic->iq_req = id->iq_min + id->i * id->iq_step;

        // Static motor keeps his last position
        ctx->interlaced_context.motor_static->iq_req = get_iq_command(&motor_static->runtime_iq_ctrl,
                                                                  motor_static->fb.pos,
                                                                  motor_static->fb.speed,
                                                                  motor_static->pos_target,
                                                                  0.0,
                                                                  0.0);

        if (motor_dynamic->demo_count == id->start_count) {
            id->speed_final[id->motor][id->i] = 0.0;

            id->t0 = motor_dynamic->demo_count;
            id->pos_t0 = motor_dynamic->fb.pos;
            id->speed_t0 = motor_dynamic->fb.speed;
        } else if (motor_dynamic->demo_count == id->start_count + id->state_count / 2) {
            id->t1 = motor_dynamic->demo_count;
            id->pos_t1 = motor_dynamic->fb.pos;
            id->speed_t1 = motor_dynamic->fb.speed;
        } else if (motor_dynamic->demo_count == id->start_count + id->state_count) {
            id->t2 = motor_dynamic->demo_count;
            id->pos_t2 = motor_dynamic->fb.pos;
            id->speed_t2 = motor_dynamic->fb.speed;

            id->b_J[id->motor][id->i] =
                ((id->t2 - id->t1) * (id->speed_t1 - id->speed_t0) -
                (id->t1 - id->t0) * (id->speed_t2 - id->speed_t1)) /
                (((id->t1 - id->t0) * (id->pos_t2 - id->pos_t1) -
                (id->t2 - id->t1) * (id->pos_t1 - id->pos_t0))) / SECS_PER_MIN;
        }

        id->speed_final[id->motor][id->i] =
            id->speed_final[id->motor][id->i] * (1.0 - id->alpha) +
            motor_dynamic->fb.speed * id->alpha;

        if (motor_dynamic->demo_count >= id->start_count + id->state_count)
            id->state = COASTING;

        break;

    case COASTING:

       ctx->interlaced_context.motor_dynamic->iq_req = 0.0;

       ctx->interlaced_context.motor_static->iq_req = get_iq_command(&motor_static->runtime_iq_ctrl,
                                                                  motor_static->fb.pos,
                                                                  motor_static->fb.speed,
                                                                  motor_static->pos_target,
                                                                  0.0,
                                                                  0.0);
       if (motor_dynamic->fb.speed < 200.0) {
            id->state = STOPPING;
            id->start_count = motor_dynamic->demo_count + 1;
            motor_dynamic->pos_target = ceil(motor_dynamic->fb.pos) + 1.0;
            motor_dynamic->speed_target = motor_dynamic->fb.speed;

            compute_trap_traj(&motor_dynamic->traj,
                      motor_dynamic->demo_count,
                      motor_dynamic->pos_target,
                      motor_dynamic->fb.pos,
                      motor_dynamic->fb.speed,
                      motor_dynamic->speed_target,
                      ctx->app_period_ns,
                      &motor_dynamic->params);
       }

       break;

    case STOPPING:

        evaluate_trap_traj(&motor_dynamic->traj,
                       &motor_dynamic->pos_target,
                       &motor_dynamic->speed_target,
                       &motor_dynamic->accel_target,
                       motor_dynamic->demo_count);

        ctx->interlaced_context.motor_dynamic->iq_req = get_iq_command(&motor_dynamic->runtime_iq_ctrl,
                                                                   motor_dynamic->fb.pos,
                                                                   motor_dynamic->fb.speed,
                                                                   motor_dynamic->pos_target,
                                                                   motor_dynamic->speed_target,
                                                                   motor_dynamic->accel_target);

        // Static motor keeps his last position
        ctx->interlaced_context.motor_static->iq_req = get_iq_command(&motor_static->runtime_iq_ctrl,
                                                                  motor_static->fb.pos,
                                                                  motor_static->fb.speed,
                                                                  motor_static->pos_target,
                                                                  0.0,
                                                                  0.0);

        if (motor_dynamic->demo_count > id->start_count + id->state_count) {
             id->i++;
             if (id->i >= id->i_max) {
                 // Switch controller types
                 iq_controller_init(&motor_dynamic->runtime_iq_ctrl, CTRL_MODE_POSITION,
                           &motor_dynamic->params);
                 iq_controller_init(&motor_static->runtime_iq_ctrl, CTRL_MODE_TRAJECTORY,
                           &motor_static->params);

                 id->id[id->motor] = motor_dynamic->id;

                 switch_motor_alternation_interlaced(ctx);

                 id->motor++;
                 if (id->motor >= IDENTIFICATION_MAX_MOTOR) {
                     id->state = DONE;
                 } else {
                     id->state = ACCELERATING;
                     id->i = 0;
                     id->start_count = motor_static->demo_count;
                }
             } else {
                 id->state = ACCELERATING;
                 id->start_count = motor_dynamic->demo_count + 1;
             }
        }

        break;

    case DONE:
        ctx->interlaced_context.motor_dynamic->iq_req = 0.0;
        ctx->interlaced_context.motor_static->iq_req = 0.0;

        for (j = 0; j < IDENTIFICATION_MAX_MOTOR; j++) {
            INF("motor%u:%u\n", (id->id[j] >> 8) & 0xff, id->id[j] & 0xff);

            INF("I(A)     w_final(rpm):\n");

            for (i = 0; i < id->i_max; i++)
                INF("%8f %8f\n", id->iq_min + i * id->iq_step, id->speed_final[j][i]);

            INF("\n");
            INF("I(A)     b/J(1/s):\n");
            for (i = 0; i < id->i_max; i++)
                INF("%8f %8f\n", id->iq_min + i * id->iq_step, id->b_J[j][i]);
        }

        break;
    }

    motor_dynamic->demo_count++;

    return CTRL_STRAT_OK;
}

/* -----  STATS  ----- */

static void control_strategy_stats_print(void *data)
{
    struct stats_control_strategy *stats = data;
    struct control_strategy_ctx *ctx = container_of(data, struct control_strategy_ctx, stats_snap);

    INF("ctx(%x):\n", ctx);
    INF("  state            : %s\n", strategy_states_names[stats->state]);
    INF("  current strategy : %s\n", strategy_names[stats->current_strategy]);
    INF("  old strategy     : %s\n", strategy_names[stats->old_strategy]);

    stats->pending = false;
}

static void control_strategy_motor_stats_print(void *data)
{
    struct stats_controlled_motor *stats = data;
    struct controlled_motor_ctx *ctx = container_of(data, struct controlled_motor_ctx, stats_snap);

    stats_compute(&stats->pos_err_deg);

    INF("ctx(%x) io_device id: %hu, motor id: %hu\n", ctx, ctx->id >> 8, ctx->id & 0xff);
    INF("  startup offset : %f\n", stats->startup_offset);
    INF("  pos real       : %f\n", stats->pos_real);
    INF("  pos target     : %f\n", stats->pos_target);
    INF("  speed real     : %f\n", stats->speed_real);
    INF("  errors margin: %u, margin stop: %u\n", stats->err_margin, stats->err_margin_stop);
    stats_print(&stats->pos_err_deg);
    hist_print(&stats->pos_err_deg_hist);

    if (stats->err_margin)
        INF("  last margin error: %f\n", stats->last_margin_error);

    stats->pending = false;
}

static void __motor_stats_send(void *data)
{
    struct control_strategy_ctx *ctx = data;
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);
        network_stats_send(ctx->net_stats_ctx, &motor->net_stats);
    }
}

static void motor_stats_send(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    if (ctx->net_stats_pending)
        return;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motor->net_stats.id = motor->id;
        motor->net_stats.seqid = motor->seqid_stats++;
        motor->net_stats.demo_count = motor->absolute_time_beginning + motor->demo_count * 250;
        motor->net_stats.pos_real = motor->fb.pos - motor->startup_offset;
        motor->net_stats.pos_target = motor->pos_target;
        motor->net_stats.speed_real = motor->fb.speed;
        motor->net_stats.speed_target_ff = motor->speed_target;
        motor->net_stats.speed_out_pos_err = motor->runtime_iq_ctrl.speed_from_pos_err;
        motor->net_stats.iq_req = motor->iq_req;
        motor->net_stats.iq_real = motor->fb.iq_meas;
        motor->net_stats.id_real = motor->fb.id_meas;
        motor->net_stats.uq_applied_fb = motor->fb.uq_applied;
        motor->net_stats.dc_bus_fb = motor->fb.dc_bus;
    }

    ctx->net_stats_pending = true;

    if (STATS_Async(__motor_stats_send, ctx) != pdTRUE)
        ctx->net_stats_pending = false;
}

static void control_strategy_set_state(struct control_strategy_ctx *ctx, control_strategy_state_t new_state)
{
    ctx->state = new_state;
}

static void control_strategy_error_cb(struct control_strategy_ctx *ctx, int err)
{
    if (ctx->err_cb)
        ctx->err_cb(ctx->err_cb_user_data, err);
}

static void control_strategy_prepare(struct control_strategy_ctx *ctx)
{
    ctrl_strategy_return_codes_t rc;

    rc = ctx->ops[ctx->current_strategy].prepare(ctx);

    if (rc == CTRL_STRAT_OK)
        control_strategy_set_state(ctx, STARTUP);
    else if (rc == CTRL_STRAT_ERROR)
        control_strategy_error_cb(ctx, -1);
}

static void control_strategy_startup(struct control_strategy_ctx *ctx)
{
    ctrl_strategy_return_codes_t rc;

    rc = ctx->ops[ctx->current_strategy].startup(ctx);

    if (rc == CTRL_STRAT_OK)
        control_strategy_set_state(ctx, INIT);
    else if (rc == CTRL_STRAT_ERROR)
        control_strategy_error_cb(ctx, -1);
}

static void control_strategy_init(struct control_strategy_ctx *ctx)
{
    ctrl_strategy_return_codes_t rc;

    rc = ctx->ops[ctx->current_strategy].init(ctx);

    if (rc == CTRL_STRAT_OK)
        control_strategy_set_state(ctx, CONTROL);
    else if (rc == CTRL_STRAT_ERROR)
        control_strategy_error_cb(ctx, -1);
}

static void control_strategy_loop(struct control_strategy_ctx *ctx)
{
    ctrl_strategy_return_codes_t rc;

    // Send stats after updating feedback and before updating targets
    motor_stats_send(ctx);

    // Apply control strategy
    rc = ctx->ops[ctx->current_strategy].loop(ctx);

    if (rc == CTRL_STRAT_ERROR)
        control_strategy_error_cb(ctx, -1);
}

static void control_strategy_reset(struct control_strategy_ctx *ctx)
{
    ctrl_strategy_return_codes_t rc;

    rc = ctx->ops[ctx->current_strategy].reset(ctx);

    if (rc == CTRL_STRAT_OK)
        control_strategy_set_state(ctx, PREPARE);
    else if (rc == CTRL_STRAT_ERROR)
        control_strategy_error_cb(ctx, -1);
}

/* -----  API  ----- */

int control_strategy_context_init(struct control_strategy_ctx **ctx, control_strategies_t first_strategy,
                                  unsigned int app_period_ns)
{
    if (control_strategy_h != NULL) {
        ERR("Strategy context already initialized\n");
        goto err;
    }

    control_strategy_h = &control_strategy;

    slist_head_init(&control_strategy_h->motor_list);

    control_strategy_h->app_period_ns = app_period_ns;
    control_strategy_h->current_strategy = first_strategy;
    control_strategy_h->old_strategy = first_strategy;
    control_strategy_h->num_strategies = MAX_NUM_CONTROL_STRATEGIES;
    control_strategy_h->ops[SYNCHRONIZED].prepare = strategy_generic_prepare;
    control_strategy_h->ops[SYNCHRONIZED].startup = strategy_generic_startup;
    control_strategy_h->ops[SYNCHRONIZED].init = strategy_synchronized_init;
    control_strategy_h->ops[SYNCHRONIZED].loop = strategy_synchronized_loop;
    control_strategy_h->ops[SYNCHRONIZED].reset = strategy_synchronized_reset;
    control_strategy_h->ops[FOLLOW].prepare = strategy_generic_prepare;
    control_strategy_h->ops[FOLLOW].startup = strategy_generic_startup;
    control_strategy_h->ops[FOLLOW].init = strategy_follow_init;
    control_strategy_h->ops[FOLLOW].loop = strategy_follow_loop;
    control_strategy_h->ops[FOLLOW].reset = strategy_follow_reset;
    control_strategy_h->ops[HOLD_INDEX].prepare = strategy_generic_prepare;
    control_strategy_h->ops[HOLD_INDEX].startup = strategy_generic_startup;
    control_strategy_h->ops[HOLD_INDEX].init = strategy_hold_index_init;
    control_strategy_h->ops[HOLD_INDEX].loop = strategy_hold_index_loop;
    control_strategy_h->ops[HOLD_INDEX].reset = strategy_hold_index_reset;
    control_strategy_h->ops[INTERLACED].prepare = strategy_interlaced_prepare;
    control_strategy_h->ops[INTERLACED].startup = strategy_interlaced_startup;
    control_strategy_h->ops[INTERLACED].init = strategy_interlaced_init;
    control_strategy_h->ops[INTERLACED].loop = strategy_interlaced_loop;
    control_strategy_h->ops[INTERLACED].reset = strategy_interlaced_reset;
    control_strategy_h->ops[STOP].prepare = strategy_stop_prepare;
    control_strategy_h->ops[STOP].startup = strategy_stop_startup;
    control_strategy_h->ops[STOP].init = strategy_stop_init;
    control_strategy_h->ops[STOP].loop = strategy_stop_loop;
    control_strategy_h->ops[STOP].reset = strategy_stop_reset;
    control_strategy_h->ops[IDENTIFY].prepare = strategy_interlaced_prepare;
    control_strategy_h->ops[IDENTIFY].startup = strategy_interlaced_startup;
    control_strategy_h->ops[IDENTIFY].init = strategy_identification_init;
    control_strategy_h->ops[IDENTIFY].loop = strategy_identification_loop;
    control_strategy_h->ops[IDENTIFY].reset = strategy_interlaced_reset;
    control_strategy_h->state = RESET;

    if (network_stats_open(&control_strategy_h->net_stats_ctx) < 0) {
        ERR("Failed to open network stats socket\n");
        goto err;
    }

    *ctx = control_strategy_h;

    INF("Strategy successfuly initialized\n");

    return 0;

err:
    return -1;
}

void control_strategy_stats_dump(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    if (!ctx->stats_snap.pending) {
        ctx->stats.state = ctx->state;
        ctx->stats.old_strategy = ctx->old_strategy;
        ctx->stats.current_strategy = ctx->current_strategy;

        memcpy(&ctx->stats_snap, &ctx->stats, sizeof(struct stats_control_strategy));
        ctx->stats_snap.pending = true;

        if (STATS_Async(control_strategy_stats_print, &ctx->stats_snap) != pdTRUE)
            ctx->stats_snap.pending = false;
    }

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        if (motor->stats_snap.pending)
            continue;

        motor->stats.iq_req = motor->iq_req;
        motor->stats.pos_real = motor->fb.pos;
        motor->stats.pos_target = motor->pos_target;
        motor->stats.speed_real = motor->fb.speed;
        motor->stats.startup_offset = motor->startup_offset;

        memcpy(&motor->stats_snap, &motor->stats, sizeof(struct stats_controlled_motor));
        motor->stats_snap.pending = true;

        stats_reset(&motor->stats.pos_err_deg);

        // Print motor data on serial interface
        if (STATS_Async(control_strategy_motor_stats_print, &motor->stats_snap) != pdTRUE)
            motor->stats_snap.pending = false;
    }
}

struct controlled_motor_ctx *control_strategy_register_motor(struct control_strategy_ctx *ctx, uint16_t io_device_id, uint16_t motor_id, uint64_t time)
{
    struct controlled_motor_ctx *new_motor = pvPortMalloc(sizeof(struct controlled_motor_ctx));
    if (!new_motor) {
        ERR("Unable to allocate controlled motor context\n");
        goto err;
    }

    memset(new_motor, 0, sizeof(*new_motor));

    reset_trap_traj(&new_motor->traj);
    reset_scenario(&new_motor->scenario);
    new_motor->id = (io_device_id << 8) | motor_id;
    new_motor->absolute_time_beginning = time;

    // Init motor stats
    stats_init(&new_motor->stats.pos_err_deg, 31, "pos err", NULL);
    hist_init(&new_motor->stats.pos_err_deg_hist, 180, 1);

    motor_params_init(&new_motor->params, new_motor->id);

    // Initialize startup controller
    iq_controller_init(&new_motor->startup_iq_ctrl, CTRL_MODE_TRAJECTORY,
                       &new_motor->params);

    slist_add_head(&ctx->motor_list, &new_motor->node);
    ctx->num_motors++;

    INF("Registered motor with : \n");
    INF("    IO device ID : %hu\n", io_device_id);
    INF("    Motor ID     : %hu\n", motor_id);
    INF("    Internal ID  : %hu\n", new_motor->id);

    return new_motor;

err:
    return NULL;
}

int control_strategy_set_feedback(struct controlled_motor_ctx *motor, struct motor_feedback *feedback)
{
    memcpy(&motor->fb, feedback, sizeof(struct motor_feedback));

    return 0;
}

void control_strategy_reset_iq(struct control_strategy_ctx *ctx)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;

    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);
        motor->iq_req = 0.0;
    }
}

float control_strategy_get_iq(struct controlled_motor_ctx *motor)
{
    return motor->iq_req;
}

void control_strategy_state_machine(struct control_strategy_ctx *ctx)
{
    switch (ctx->state) {
    case PREPARE:
        control_strategy_prepare(ctx);
        break;
    case STARTUP:
        control_strategy_startup(ctx);
        break;
    case INIT:
        control_strategy_init(ctx);
        break;
    case CONTROL:
        control_strategy_loop(ctx);
        break;
    case RESET:
        control_strategy_reset(ctx);
        break;
    default:
        break;
    }
}

void control_strategy_get_motor_monitoring(struct control_strategy_ctx *ctx, struct monitoring_msg_motor *motors_data_arr, uint32_t num_motor_monitored)
{
    struct slist_node *entry;
    struct controlled_motor_ctx *motor;
    uint32_t i = 0;

    // We send stats after updating feedback and before updating targets
    slist_for_each (&ctx->motor_list, entry) {
        motor = container_of(entry, struct controlled_motor_ctx, node);

        motors_data_arr[i].id = motor->id;
        motors_data_arr[i].pos_error = motor->stats.pos_err_max;

        // Reset pos err stat
        motor->stats.pos_err_max = 0;

        i++;
        if (i >= num_motor_monitored)
            return;
    }
}

int control_strategy_reset_strategy(struct control_strategy_ctx *ctx)
{
    control_strategy_set_state(ctx, RESET);

    return 0;
}

int control_strategy_previous_strategy(struct control_strategy_ctx *ctx)
{
    control_strategies_t tmp;

    tmp = ctx->current_strategy;
    ctx->current_strategy = ctx->old_strategy;
    ctx->old_strategy = tmp;

    control_strategy_set_state(ctx, RESET);

    return 0;
}

int control_strategy_next_strategy(struct control_strategy_ctx *ctx)
{
    ctx->old_strategy = ctx->current_strategy;
    ctx->current_strategy++;
    ctx->current_strategy %= ctx->num_strategies;

    control_strategy_set_state(ctx, RESET);

    return 0;
}

int control_strategy_set_strategy(struct control_strategy_ctx *ctx, control_strategies_t new_strategy)
{
    if (ctx->current_strategy != new_strategy) {
        ctx->old_strategy = ctx->current_strategy;
        ctx->current_strategy = new_strategy;

        control_strategy_set_state(ctx, RESET);
    }

    return 0;
}

control_strategies_t control_strategy_get_strategy(struct control_strategy_ctx *ctx)
{
    return ctx->current_strategy;
}

void control_strategy_set_error_callback(struct control_strategy_ctx *ctx, void (*callback)(void *, int), void *user_data)
{
    ctx->err_cb = callback;
    ctx->err_cb_user_data = user_data;
}
