/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RT_LATENCY_H_
#define _RT_LATENCY_H_

#include "os/semaphore.h"

#include "stats.h"

#include "rt_tc_setup.h"

/*
 * Symbol definitions:
 *
 * WITH_IRQ_LOAD:     Add an extra IRQ load thread
 * WITH_CPU_LOAD:     Add CPU load in the lower priority task
 */
#define RT_LATENCY_WITH_IRQ_LOAD             (1 << 1)
#define RT_LATENCY_WITH_CPU_LOAD             (1 << 2)

static inline int rt_latency_get_tc_load(int test_case_id)
{
    int mask = 0;

    switch (test_case_id) {
        case 1:
            break;
        case 2:
            mask |= RT_LATENCY_WITH_CPU_LOAD;
            break;
        case 3:
            mask |= RT_LATENCY_WITH_IRQ_LOAD;
            break;
        case 4:
            mask |= RT_LATENCY_WITH_CPU_LOAD;
            break;
        case 5:
            mask |= RT_LATENCY_WITH_CPU_LOAD;
            /* TODO: Add command to trigger Linux Load */
            break;
        case 6:
            mask |= RT_LATENCY_WITH_CPU_LOAD;
            break;
        case 7:
            mask |= RT_LATENCY_WITH_CPU_LOAD;
            /* TODO: Place code in OCRAM (outer cacheable) */
            /* TODO: Add command to trigger Linux Load */
            break;
        default:
            mask = -1;
            break;
    }

    return mask;
}

struct rt_latency_ctx {
	const void *dev;
	const void *irq_load_dev;

  int tc_load; /* bitmask of the above RT_LATENCY_WITH_xxx test case load conditions */

	os_sem_t semaphore; /* used to wake the thread up from IRQ callback */
	os_sem_t irq_load_sem; /* used to wake the thread up from IRQ load handler */

	uint64_t time_irq;
	uint32_t time_prog;

	struct stats irq_delay;
	struct hist irq_delay_hist;

	struct stats irq_to_sched;
	struct hist irq_to_sched_hist;
};

int rt_latency_init(const void *dev,
		const void *irq_load_dev, struct rt_latency_ctx *ctx);

int rt_latency_test(struct rt_latency_ctx *ctx);

void print_stats(struct rt_latency_ctx *ctx);
void cpu_load(void);
#ifdef WITH_INVD_CACHE
void cache_inval(void);
#endif

#endif /* _RT_LATENCY_H_ */
