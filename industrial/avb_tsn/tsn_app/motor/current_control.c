/*
 * Copyright 2019, 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "current_control.h"

#include "motor_params.h"
#include "rtos_apps/types.h"

/** Position P Controller
 *
 * \return Speed command in rpm
 * \param iq_ctrl control parameters struct
 * \param pos_real current position of the motor (in revolutions)
 * \param pos_cmd position target (in revolutions)
 */
static float position_control(struct iq_control *iq_ctrl, float pos_real, float pos_cmd)
{
    float pos_err = pos_cmd - pos_real;
    float speed_out = pos_err * iq_ctrl->pos_kp;

    if (speed_out >= iq_ctrl->speed_max) {
        speed_out = iq_ctrl->speed_max;
    } else if (speed_out <= -iq_ctrl->speed_max) {
        speed_out = -iq_ctrl->speed_max;
    }

    return speed_out;
}

/** Speed PI Controller
 *
 * \return Iq command in amps
 * \param iq_ctrl control parameters struct
 * \param speed_real current speed of the motor (in rpm)
 * \param speed_cmd speed target (in rpm)
 */
static float speed_control(struct iq_control *iq_ctrl, float speed_real, float speed_cmd)
{
    float speed_error, iq_out;

    /* Scaling conversion */
    speed_cmd = speed_cmd / iq_ctrl->speed_angular_scale;
    speed_real = speed_real / iq_ctrl->speed_angular_scale;

    speed_error = speed_cmd - speed_real;

    iq_out = speed_error * iq_ctrl->speed_kp + (iq_ctrl->speed_integral_error + speed_error) * iq_ctrl->speed_ki;

    /* Anti wind-up */
    if (iq_out >= iq_ctrl->iq_max) {
        if (speed_error > 0.0) {
            goto exit;
        }
    } else if (iq_out <= iq_ctrl->iq_min) {
        if (speed_error < 0.0) {
            goto exit;
        }
    }

    iq_ctrl->speed_integral_error = iq_ctrl->speed_integral_error + speed_error;

exit:
    return iq_out;
}

/** Iq feedforward
 *
 * \return Iq command in amps
 * \param iq_ctrl control parameters struct
 * \param speed_real current speed of the motor (in rpm)
 * \param accel_cmd acceleration target (in rpm/s)
 */
static float iq_feedforward(struct iq_control *iq_ctrl, float speed_real, float accel_cmd)
{
    return iq_ctrl->J * accel_cmd + iq_ctrl->b * speed_real + iq_ctrl->Tm;
}

/** Iq controller struct initialization
 *
 * \return None
 * \param iq_ctrl control parameters struct
 * \param ctrl_mode control mode of the motor
 * \param params motor control params
 */
void iq_controller_init(struct iq_control *iq_ctrl, enum control_mode ctrl_mode, struct motor_control_params *params)
{
    iq_ctrl->ctrl_mode = ctrl_mode;
    iq_ctrl->pos_kp = params->pos_kp;
    iq_ctrl->pos_gain = params->pos_gain;
    iq_ctrl->speed_angular_scale = params->speed_angular_scale;
    iq_ctrl->speed_kp = params->speed_kp;
    iq_ctrl->speed_ki = params->speed_ki;
    iq_ctrl->speed_integral_error = 0.0;
    iq_ctrl->speed_max = params->max_vel_rpm * SECS_PER_MIN;
    iq_ctrl->speed_gain = params->speed_gain;
    iq_ctrl->iq_max = params->iq_max;
    iq_ctrl->iq_min = params->iq_min;

    iq_ctrl->J = params->J;
    iq_ctrl->b = params->b;
    iq_ctrl->Tm = params->Tm;
    iq_ctrl->ff_gain = params->ff_gain;
}

/** Compute Iq command depending on control type choosen by the user
 *
 * \return Iq command
 * \param iq_ctrl control parameters struct
 * \param pos_real current position of the motor (in revolutions)
 * \param speed_real current speed of the motor (in rpm)
 * \param pos_cmd position target (in revolutions). ONLY used in CTRL_MODE_POSITION
 * \param speed_cmd speed target (in rpm). ONLY used in CTRL_MODE_SPEED
 * \param accel_cmd accel target (in rpm/s)
 */
float get_iq_command(struct iq_control *iq_ctrl, float pos_real, float speed_real,
                     float pos_cmd, float speed_cmd, float accel_cmd)
{
    float iq_cmd;

    switch (iq_ctrl->ctrl_mode) {
    case CTRL_MODE_TRAJECTORY:
        /* Compute speed with position feedback and feedforward gains */
        iq_ctrl->speed_from_pos_err = position_control(iq_ctrl, pos_real, pos_cmd);
        speed_cmd += iq_ctrl->pos_gain * iq_ctrl->speed_from_pos_err;
        break;
    case CTRL_MODE_POSITION:
        /* Compute speed with position feedback and basic gains */
        speed_cmd = position_control(iq_ctrl, pos_real, pos_cmd);
        break;

    case CTRL_MODE_SPEED:
    default:
        break;
    }

    iq_cmd = iq_ctrl->ff_gain * iq_feedforward(iq_ctrl, speed_real, accel_cmd);
    iq_cmd += iq_ctrl->speed_gain * speed_control(iq_ctrl, speed_real, speed_cmd);

    if (iq_cmd >= iq_ctrl->iq_max) {
        iq_cmd = iq_ctrl->iq_max;
    } else if (iq_cmd <= iq_ctrl->iq_min) {
        iq_cmd = iq_ctrl->iq_min;
    }

    return iq_cmd;
}
