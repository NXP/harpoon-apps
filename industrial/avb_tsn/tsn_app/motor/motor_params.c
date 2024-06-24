/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "genavb/helpers.h"

#include "log.h"
#include "m1_pmsm_appconfig.h"
#include "os/math.h"
#include "motor_params.h"
#include "storage.h"

#define MIN_ACCEL_RPM_P_S	1000.0
#define MIN_VEL_RPM		60.0

static void motor_params_dump(struct motor_control_params *params, unsigned int id)
{
    INF("motor%u:%u params:\n", (id >> 8) & 0xff, id & 0xff);
    INF("vel max: %f (rpm)\n", params->max_vel_rpm);
    INF("acc max: %f (rpm/s)\n", params->max_accel_rpm_p_s);

    INF("J:       %f (A.s/rpm)\n", params->J);
    INF("b:       %f (A/rpm)\n", params->b);
    INF("Tm:      %f (A)\n", params->Tm);
}

static void motor_params_check(struct motor_control_params *params)
{
    float ratio = params->max_accel_rpm_p_s / params->max_vel_rpm;

    if (params->max_accel_rpm_p_s < MIN_ACCEL_RPM_P_S)
        params->max_accel_rpm_p_s = MIN_ACCEL_RPM_P_S;

    if (params->max_vel_rpm < MIN_VEL_RPM)
        params->max_vel_rpm = MIN_VEL_RPM;

    if (params->J < 0.0)
        params->J = MOTOR_J;

    if (params->b < 0.0)
        params->b = MOTOR_b;

    if (params->Tm < 0.0)
        params->Tm = MOTOR_Tm;

    if (params->iq_max < params->J * params->max_accel_rpm_p_s + params->b * params->max_vel_rpm + params->Tm) {
        ratio = params->max_accel_rpm_p_s / params->max_vel_rpm;

        params->max_vel_rpm = (params->iq_max - params->Tm) / (params->J * ratio + params->b);
        params->max_accel_rpm_p_s = ratio * params->max_vel_rpm;
    }

    if (-params->iq_min < params->J * params->max_accel_rpm_p_s + params->b * params->max_vel_rpm - params->Tm)
    {
        ratio = params->max_accel_rpm_p_s / params->max_vel_rpm;

        params->max_vel_rpm = (-params->iq_min + params->Tm) / (params->J * ratio + params->b);
        params->max_accel_rpm_p_s = ratio * params->max_vel_rpm;
    }

    if (params->max_accel_rpm_p_s < MIN_ACCEL_RPM_P_S || params->max_vel_rpm < MIN_VEL_RPM) {
        params->max_accel_rpm_p_s = MIN_ACCEL_RPM_P_S;
        params->max_vel_rpm = MIN_VEL_RPM;
        params->J = MOTOR_J;
        params->b = MOTOR_b;
        params->Tm = MOTOR_Tm;
    }
}

static void motor_params_default(struct motor_control_params *params)
{
    params->pos_kp = NETCTRL_POS_KP;
    params->pos_gain = POS_GAIN;

    params->speed_angular_scale = (60.0F / (M1_MOTOR_PP * 2.0F * M_PI));
    params->speed_kp = NETCTRL_SPEED_KP;
    params->speed_ki = NETCTRL_SPEED_KI;
    params->max_vel_rpm = MAX_VEL_RPM;
    params->speed_gain = SPEED_GAIN;

    params->iq_max = M1_SPEED_LOOP_HIGH_LIMIT;
    params->iq_min = M1_SPEED_LOOP_LOW_LIMIT;

    params->max_accel_rpm_p_s = MAX_ACCEL_RPM_P_S;

    params->J = MOTOR_J;
    params->b = MOTOR_b;
    params->Tm = MOTOR_Tm;
    params->ff_gain = FF_GAIN;
}

void motor_params_init(struct motor_control_params *params, unsigned int id)
{
    char buf[20];

    motor_params_default(params);

    h_snprintf(buf, 20, "/tsn_app/motor%u:%u", (id >> 8) & 0xff, id & 0xff);

    if (storage_cd(buf, true))
        goto out;

    storage_read_float("max_vel", &params->max_vel_rpm);
    storage_read_float("max_accel", &params->max_accel_rpm_p_s);

    storage_read_float("J", &params->J);
    storage_read_float("b", &params->b);
    storage_read_float("Tm", &params->Tm);

    storage_cd("-", true);

out:
    motor_params_check(params);

    motor_params_dump(params, id);
}
