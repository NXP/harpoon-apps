/*
 * Copyright 2019, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CONTROL_STRATEGIES_H_
#define _CONTROL_STRATEGIES_H_

#include <stdint.h>
#include <stdbool.h>
#include "current_control.h"
#include "scenarios.h"
#include "motor_control.h"
#include "monitoring_stats.h"
#include "tsn_tasks_config.h"

#define MAX_NUM_CONTROL_STRATEGIES 6

/* Forward declaration of private types */
struct controlled_motor_ctx;
struct control_strategy_ctx;

int control_strategy_context_init(struct control_strategy_ctx **ctx, control_strategies_t first_strategy, unsigned int app_period_ns);
void control_strategy_stats_dump(struct control_strategy_ctx *ctx);
struct controlled_motor_ctx *control_strategy_register_motor(struct control_strategy_ctx *ctx, uint16_t io_device_id, uint16_t motor_id, uint64_t time);
int control_strategy_set_feedback(struct controlled_motor_ctx *motor, struct motor_feedback *feedback);
float control_strategy_get_iq(struct controlled_motor_ctx *motor);
void control_strategy_reset_iq(struct control_strategy_ctx *ctx);
void control_strategy_state_machine(struct control_strategy_ctx *ctx);
void control_strategy_get_motor_monitoring(struct control_strategy_ctx *ctx, struct monitoring_msg_motor *motors_data_arr, uint32_t num_motor_monitored);
int control_strategy_reset_strategy(struct control_strategy_ctx *ctx);
int control_strategy_previous_strategy(struct control_strategy_ctx *ctx);
int control_strategy_next_strategy(struct control_strategy_ctx *ctx);
int control_strategy_set_strategy(struct control_strategy_ctx *ctx, control_strategies_t new_strategy);
control_strategies_t control_strategy_get_strategy(struct control_strategy_ctx *ctx);
void control_strategy_set_error_callback(struct control_strategy_ctx *ctx, void (*callback)(void *, int), void *user_data);

#endif /* _CONTROL_STRATEGIES_H_ */
