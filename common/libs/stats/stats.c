/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/stdio.h"

#include "stats.h"

#define INF			os_printf
#define VERBOSE_INFO		(2)

static unsigned int app_log_level = VERBOSE_INFO;

void stats_reset(struct stats *s)
{
    s->current_count = 0;
    s->current_min = 0x7fffffff;
    s->current_mean = 0;
    s->current_max = -0x7fffffff;
    s->current_ms = 0;
}

/** Example function to be passed to stats_init
 * Usage:
 * stats_init(s, log2_size, "your string", print_stats);
 */
void stats_print(struct stats *s)
{
    INF("stats(%p) %s min %d mean %d max %d rms^2 %llu stddev^2 %llu absmin %d absmax %d\n\r",
        s, s->name, s->min, s->mean, s->max, s->ms, s->variance, s->abs_min, s->abs_max);
}

/** Update stats with a given sample.
 * @s: handler for the stats being monitored
 * @val: sample to be added
 *
 * This function adds a sample to the set.
 * If the number of accumulated samples is equal to the requested size of the set, the following indicators will be computed:
 *    . minimum observed value,
 *    . maximum observed value,
 *    . mean value,
 *    . square of the RMS (i.e. mean of the squares)
 *    . square of the standard deviation (i.e. variance)
 */
void stats_update(struct stats *s, int32_t val)
{
    s->current_count++;

    s->current_mean += val;
    s->current_ms += (int64_t)val * val;

    if (val < s->current_min) {
        s->current_min = val;
        if (val < s->abs_min)
            s->abs_min = val;
    }

    if (val > s->current_max) {
        s->current_max = val;
        if (val > s->abs_max)
            s->abs_max = val;
    }

    if (s->current_count == (1U << s->log2_size)) {
        s->ms = s->current_ms >> s->log2_size;
        s->variance = s->ms - ((s->current_mean * s->current_mean) >> (2 * s->log2_size));
        s->mean = s->current_mean >> s->log2_size;

        s->min = s->current_min;
        s->max = s->current_max;

        if (s->func)
            s->func(s);

        stats_reset(s);
    }
}

/** Compute current stats event if set size hasn't been reached yet.
 * @s: handler for the stats being monitored
 *
 * This function computes current statistics for the stats:
 *    . minimum observed value,
 *    . maximum observed value,
 *    . mean value,
 *    . square of the RMS (i.e. mean of the squares)
 *    . square of the standard deviation (i.e. variance)
 */
void stats_compute(struct stats *s)
{
    if (s->current_count) {
        s->ms = s->current_ms / s->current_count;
        s->variance = s->ms - (s->current_mean * s->current_mean) / ((int64_t)s->current_count * s->current_count);
        s->mean = s->current_mean / s->current_count;
    } else {
        s->mean = 0;
        s->ms = 0;
        s->variance = 0;
    }

    s->min = s->current_min;
    s->max = s->current_max;
}

int hist_init(struct hist *hist, unsigned int n_slots, unsigned slot_size)
{
    /* One extra slot for last bucket.
     */
    if ((n_slots + 1) > MAX_SLOTS)
        return -1;

    hist->n_slots = n_slots + 1;
    hist->slot_size = slot_size;

    return 0;
}

void hist_update(struct hist *hist, unsigned int value)
{
    unsigned int slot = value / hist->slot_size;

    if (slot >= hist->n_slots)
        slot = hist->n_slots - 1;

    hist->slots[slot]++;
}

void hist_reset(struct hist *hist)
{
    int i;

    for (i = 0; i < (hist->n_slots + 1); i++)
        hist->slots[i] = 0;
}

void hist_print(struct hist *hist)
{
    int i;

    INF("n_slot %d slot_size %d \n\r", hist->n_slots, hist->slot_size);

    if (app_log_level >= VERBOSE_INFO) {
        for (i = 0; i < (hist->n_slots + 1); i++)
            os_printf("%u ", hist->slots[i]);
        os_printf("\n\r");
    }
}
