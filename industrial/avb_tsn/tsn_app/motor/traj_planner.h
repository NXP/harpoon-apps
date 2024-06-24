/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TRAJ_PLANNER_H_
#define _TRAJ_PLANNER_H_

#include <stdint.h>
#include <stdbool.h>

struct traj_trapez {
    uint32_t cycles_accel;
    uint32_t cycles_constant_vel;
    uint32_t cycles_total;
    uint32_t start_cycle;
    float ff_current;
    float pos_init;
    float speed_init;
    float pos_target;
    float sign_traj;
    float y_end_accel;
    float accel_max;
    float speed_max;
    float loop_freq;
    bool reset_flag;
};

struct motor_control_params;

void compute_trap_traj(struct traj_trapez *traj, uint32_t start_cycle, float pos_target,
                       float actual_pos, float actual_speed, float speed_max, unsigned int app_period_ns, struct motor_control_params *params);
void check_trap_traj(struct traj_trapez *traj);
void evaluate_trap_traj(struct traj_trapez *traj, float *step_pos, float *step_speed, float *step_accel, uint32_t cycle);
void reset_trap_traj(struct traj_trapez *traj);

#endif /* _TRAJ_PLANNER_H_ */
