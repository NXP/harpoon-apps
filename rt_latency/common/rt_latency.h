/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RT_LATENCY_H_
#define _RT_LATENCY_H_

#include "os/semaphore.h"
#include "stats.h"

/* Time for Counter Alarm timeout (us) */
#define COUNTER_PERIOD_US_VAL               (20000)

#define CACHE_INVAL_PERIOD_MS                 (100)

/*
 * Symbol definitions:
 *
 * WITH_IRQ_LOAD:     Add an extra IRQ load thread
 * WITH_CPU_LOAD:     Add CPU load in the lower priority task
 * WITH_CPU_LOAD_SEM: Add Semaphore load in CPU load thread
 * WITH_INVD_CACHE:   Add cache invalidation thread
 */
#define RT_LATENCY_WITH_IRQ_LOAD             (1 << 1)
#define RT_LATENCY_WITH_CPU_LOAD             (1 << 2)
#define RT_LATENCY_WITH_CPU_LOAD_SEM         (1 << 3)
#define RT_LATENCY_WITH_INVD_CACHE           (1 << 4)
#define RT_LATENCY_WITH_LINUX_LOAD           (1 << 5)
#define RT_LATENCY_USES_OCRAM                (1 << 6)

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
            mask |= RT_LATENCY_WITH_CPU_LOAD |
                    RT_LATENCY_WITH_CPU_LOAD_SEM;
            break;
        case 5:
            mask |= RT_LATENCY_WITH_CPU_LOAD |
                    RT_LATENCY_WITH_LINUX_LOAD;
            break;
        case 6:
            mask |= RT_LATENCY_WITH_CPU_LOAD |
                    RT_LATENCY_WITH_INVD_CACHE;
            break;
        case 7:
            mask |= RT_LATENCY_WITH_CPU_LOAD |
                    RT_LATENCY_WITH_INVD_CACHE |
                    RT_LATENCY_WITH_LINUX_LOAD |
                    RT_LATENCY_USES_OCRAM;
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
void cpu_load(struct rt_latency_ctx *ctx);
void cache_inval(void);

#endif /* _RT_LATENCY_H_ */
