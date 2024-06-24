/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MOTOR_SCENARIO_H_
#define _MOTOR_SCENARIO_H_

#include <stdint.h>
#include <stdbool.h>

struct scenario_block {
    float pos_target;       // in rev
    uint32_t after_delay;   // in cycles
    float speed_max;        // in rpm
    uint32_t time_to_reach; // computed by controller
};

struct scenario_ctx {
    /* Private */
    struct scenario_block *blocks;
    uint16_t blocks_iterator;
    unsigned int scenario_size;
    unsigned int app_period_ns;

    /* Public */
    float pos_target;       // in rev
    uint32_t time_to_reach; // computed by controller
    uint32_t after_delay;   // in cycles
    float speed_max;        // in rpm
};

enum {
    SCENARIO_SYNCHRONIZED,
    SCENARIO_SYNCHRONIZED_REVERSE,
    SCENARIO_INTERLACED,
    SCENARIO_TEST,
};

int scenario_init(struct scenario_ctx *ctx, int id_scenario, unsigned int app_period_ns);
void get_scenario_block(struct scenario_ctx *ctx);
void next_block_scenario(struct scenario_ctx *ctx);
void reset_scenario(struct scenario_ctx *ctx);

#endif /* _MOTOR_SCENARIO_H_ */
