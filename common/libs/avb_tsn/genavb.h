/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _GENAVB_H_
#define _GENAVB_H_

#include "genavb/init.h"
#include "genavb/clock.h"

struct port_stats {
    const char **names;
    uint64_t *values;
    int num;
};

struct gavb_pps {
    genavb_clock_id_t clk_id;
    struct genavb_timer *t;
};

int gavb_pps_init(struct gavb_pps *pps, genavb_clock_id_t clk_id);
void gavb_pps_exit(struct gavb_pps *pps);
struct port_stats *gavb_port_stats_get(unsigned int port_id);
int gavb_port_stats_init(unsigned int port_id);
void gavb_port_stats_exit(unsigned int port_id);
int gavb_log_level(char *component_str, char *level_str);
struct genavb_handle *get_genavb_handle(void);
int gavb_stack_init(void);
int gavb_stack_exit(void);

#endif /* _GENAVB_H_ */
