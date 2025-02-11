/*
 * Copyright 2019, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MOTOR_CONTROL_API_H_
#define _MOTOR_CONTROL_API_H_

#include <stdint.h>
#include <stdbool.h>
#include "motor_control.h"

struct tsn_motor;

int mcapi_init(uint16_t id, struct tsn_motor **motor);
uint16_t mcapi_get_motor_state(struct tsn_motor *motor);
void mcapi_set_speed(struct tsn_motor *motor, float speed);
void mcapi_set_position(struct tsn_motor *motor, float pos);
int mcapi_get_motor_feedback(struct tsn_motor *motor, struct motor_feedback *feedback);
void mcapi_slowloop(struct tsn_motor *motor);
bool mcapi_motor_initialized(struct tsn_motor *motor);
void mcapi_set_iq_req(struct tsn_motor *motor, float iq);
void mcapi_set_external_control(struct tsn_motor *motor, bool state);
void mcapi_set_closed_current_loop(struct tsn_motor *motor, bool state);
void mcapi_adc_handler(void);
uint32_t mcapi_get_last_index(struct tsn_motor *motor);
uint32_t mcapi_get_missed_slow_loop(struct tsn_motor *motor);
uint32_t mcapi_get_revolution_jumps(struct tsn_motor *motor);

#endif /* _MOTOR_CONTROL_API_H_ */
