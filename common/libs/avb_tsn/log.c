/*
 * Copyright 2019, 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "log.h"

uint64_t app_log_time_s;
unsigned int app_log_level = VERBOSE_INFO;

void app_log_update_time(genavb_clock_id_t clk_id)
{
    uint64_t log_time;

    if (genavb_clock_gettime64(clk_id, &log_time) < 0)
        return;

    app_log_time_s = log_time / NSECS_PER_SEC;
}

static void __app_log_level_set(unsigned int level)
{
    if (level <= VERBOSE_DEBUG)
        app_log_level = level;
}

int app_log_level_set(char *level_str)
{
    unsigned int level;

    if (!strcmp(level_str, "crit"))
        level = VERBOSE_ERROR;
    else if (!strcmp(level_str, "err"))
        level = VERBOSE_ERROR;
    else if (!strcmp(level_str, "init"))
        level = VERBOSE_INFO;
    else if (!strcmp(level_str, "info"))
        level = VERBOSE_INFO;
    else if (!strcmp(level_str, "dbg"))
        level = VERBOSE_DEBUG;
    else
        return -1;

    __app_log_level_set(level);

    return 0;
}
