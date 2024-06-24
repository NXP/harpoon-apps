/*
 * Copyright 2019, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "traj_planner.h"
#include "math.h"
#include "hlog.h"
#include "motor_params.h"
#include "types.h"

void check_trap_traj(struct traj_trapez *traj)
{
    float t_a = traj->cycles_accel / traj->loop_freq;
    float t_v = traj->cycles_constant_vel / traj->loop_freq;

    float dX = (MAX_ACCEL_RPM_P_S / SECS_PER_MIN) * t_a * t_a + t_v * (traj->speed_max / SECS_PER_MIN);
    log_info("dX : %f\n", dX);
    log_info("Target : %f\n", traj->pos_target - traj->pos_init);
}

/** Compute trapezoidal trajectory to reach a given position target
 *
 * This function stores into a traj_trapez struct all relevant information
 * that is needed to reach a given position, such as the number of cycles
 * of acceleration, of deceleration and of constant speed.
 *
 * \return           None
 * \param traj pointer to trajectory struct
 * \param start_cycle Cycle of execution at which this function is called
 * \param pos_target position target (in revolutions)
 * \param actual_pos current position of the motor (in revolutions)
 * \param actual_speed current speed of the motor (in rpm)
 * \param accel_max maximal acceleration reachable by the motor (in rpm/s)
 * \param speed_max maximal speed reachable by the motor (in rpm)
 * \param app_period_ns main loop period (in ns) 
 */

void compute_trap_traj(struct traj_trapez *traj, uint32_t start_cycle, float pos_target, float actual_pos,
                       float actual_speed, float speed_max, unsigned int app_period_ns, struct motor_control_params *params)
{
    if (pos_target != traj->pos_target || traj->reset_flag) {
        float t_a_max, min_dist, t_a, vel_reached, t_v, accel_max;

        // Reset done
        traj->reset_flag = false;

        traj->loop_freq = NSECS_PER_SEC_F / app_period_ns;

        if (speed_max > params->max_vel_rpm)
            speed_max = params->max_vel_rpm;

        /* Speed unit conversion */
        speed_max /= SECS_PER_MIN;    // from rpm to r/s
        actual_speed /= SECS_PER_MIN; // from rpm to r/s

        // Acceleration constant
        accel_max = params->max_accel_rpm_p_s / SECS_PER_MIN; // from rpm/s to r/s2

        traj->start_cycle = start_cycle;
        traj->accel_max = accel_max;
        traj->speed_max = speed_max;

        traj->speed_init = actual_speed;
        traj->pos_init = actual_pos;
        traj->pos_target = pos_target;

        /* Difference of position */
        pos_target = (pos_target - actual_pos);

        traj->sign_traj = pos_target > 0.0F ? 1.0F : -1.0F;

        /* Maximum acceleration and deceleration time */
        t_a_max = ((speed_max - actual_speed) / accel_max);

        /* Minimal distance to go through when reaching maximum velocity */
        min_dist = 0.5 * t_a_max * (speed_max + actual_speed) + 0.5 * t_a_max * speed_max;

        if (traj->sign_traj * pos_target < min_dist) {
            /* Velocity reached during triangle profile */
            vel_reached = sqrt(
                (2.0 * accel_max * accel_max * traj->sign_traj * pos_target - accel_max * actual_speed * actual_speed) / (2 * accel_max));

            /* Cruising speed duration */
            t_v = 0.0;
        } else {
            /* Trapezoidale profile */
            vel_reached = speed_max;

            /* Cruising speed duration */
            t_v = (traj->sign_traj * pos_target - min_dist) / speed_max;
        }

        /* Actual acceleration time */
        t_a = ((vel_reached - actual_speed) / accel_max);

        traj->speed_max = vel_reached;
        traj->cycles_accel = t_a * traj->loop_freq;
        traj->cycles_constant_vel = t_v * traj->loop_freq;
        traj->cycles_total = 2.0 * traj->cycles_accel + traj->cycles_constant_vel;
        traj->y_end_accel = traj->pos_init + traj->sign_traj * (actual_speed * t_a + 0.5 * accel_max * (t_a * t_a));
    }
}

/** Compute runtime step of a trapezoidal trajectory
 *
 * \return           None
 * \param traj pointer to trajectory struct
 * \param step_pos step of position accross the trajectory (in rev)
 * \param step_speed step of speed accross the trajectory (in rpm)
 * \param step_accel step of acceleration accross the trajectory (in rpm/s)
 * \param cycle cycles executed since the relative beginning of the time
 */

void evaluate_trap_traj(struct traj_trapez *traj, float *step_pos, float *step_speed, float *step_accel, uint32_t cycle)
{
    float t;

    cycle -= traj->start_cycle;
    t = cycle / traj->loop_freq;

    if (cycle < traj->cycles_accel) {
        *step_pos = traj->pos_init + traj->sign_traj * (traj->speed_init * t + 0.5 * traj->accel_max * (t * t));

        *step_speed = traj->speed_init + traj->accel_max * t;

        *step_accel = traj->accel_max;
    } else if (cycle < traj->cycles_accel + traj->cycles_constant_vel) {
        *step_pos = traj->y_end_accel +
                    traj->sign_traj * traj->speed_max *
                        (t - (traj->cycles_accel / traj->loop_freq));

        *step_speed = traj->speed_max;

        *step_accel = 0.0;
    } else if (cycle < traj->cycles_total) {
        float t_decel = t - (traj->cycles_total / traj->loop_freq);

        *step_pos = traj->pos_target - traj->sign_traj * 0.5 * traj->accel_max *
                                         (t_decel * t_decel);

        *step_speed = (-traj->accel_max) * t_decel;

        *step_accel = -traj->accel_max;
    } else {
        *step_pos = traj->pos_target;

        *step_speed = 0.0;

        *step_accel = 0.0;

        return;
    }

    /* Unit conversion */
    *step_speed *= SECS_PER_MIN * traj->sign_traj;
    *step_accel *= SECS_PER_MIN * traj->sign_traj;
}

/** Reset trapezoidal trajectory struct
 *
 * \return           None
 * \param traj pointer to trajectory struct
 */

void reset_trap_traj(struct traj_trapez *traj)
{
    traj->reset_flag = true;
    traj->pos_target = 0.0;
    traj->cycles_accel = 0;
    traj->cycles_constant_vel = 0;
    traj->cycles_total = 0;
    traj->ff_current = 0.0;
    traj->pos_init = 0.0;
    traj->speed_init = 0.0;
    traj->pos_target = 0.0;
    traj->sign_traj = 0.0;
    traj->y_end_accel = 0.0;
    traj->accel_max = 0.0;
    traj->speed_max = 0.0;
}
