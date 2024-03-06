/*
 * Copyright 2019, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CONTROL_ALGOS_H_
#define _CONTROL_ALGOS_H_

enum control_mode {
    CTRL_MODE_TRAJECTORY,
    CTRL_MODE_POSITION,
    CTRL_MODE_SPEED
};

struct iq_control {
    enum control_mode ctrl_mode;
    float pos_kp;
    float pos_gain;
    float speed_angular_scale;
    float speed_kp;
    float speed_ki;
    float speed_integral_error;
    float speed_from_pos_err; /* Speed derived from position error */
    float speed_max;
    float speed_gain;
    float iq_max;
    float iq_min;
    float J;
    float b;
    float Tm;
    float ff_gain;
};

struct motor_control_params;

void iq_controller_init(struct iq_control *iq_ctrl, enum control_mode ctrl_mode, struct motor_control_params *params);

float get_iq_command(struct iq_control *iq_ctrl, float pos_real, float speed_real,
                     float pos_cmd, float speed_cmd, float accel_cmd);

#endif /* _CONTROL_ALGOS_H_ */
