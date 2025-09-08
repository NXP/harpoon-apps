/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RT_LATENCY_H_
#define _RT_LATENCY_H_

#include "os/counter.h"
#include "rtos_apps/stats.h"
#include "rpmsg.h"
#include "rtos_abstraction_layer.h"

/* Time period between two statistics polling logs (seconds) */
#define STATS_PERIOD_SEC					   (1)

/* Time period between two statistics dump logs (seconds) */
#define LATENCY_STATS_PERIOD_SEC				   (10)

/* Time for counter alarm timeout (us) */
#define COUNTER_PERIOD_US_VAL			   (100)

/* Timeout to wait for timer irq (ms) */
#define COUNTER_IRQ_TIMEOUT_MS			   (10)

/* Time between two cache invalidation instructions (ms) */
#define CACHE_INVAL_PERIOD_MS				 (100)

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

enum rt_latency_test_case_id
{
	RT_LATENCY_TEST_CASE_1 = 1,
	RT_LATENCY_TEST_CASE_2,
	RT_LATENCY_TEST_CASE_3,
	RT_LATENCY_TEST_CASE_4,
	RT_LATENCY_TEST_CASE_5,
	RT_LATENCY_TEST_CASE_6,
	RT_LATENCY_TEST_CASE_7,

	RT_LATENCY_TEST_CASE_MAX,
};

static inline int rt_latency_get_tc_load(int test_case_id)
{
	int mask = 0;

	switch (test_case_id) {
	case RT_LATENCY_TEST_CASE_1:
		break;
	case RT_LATENCY_TEST_CASE_2:
		mask |= RT_LATENCY_WITH_CPU_LOAD;
		break;
	case RT_LATENCY_TEST_CASE_3:
		mask |= RT_LATENCY_WITH_IRQ_LOAD;
		break;
	case RT_LATENCY_TEST_CASE_4:
		mask |= RT_LATENCY_WITH_CPU_LOAD |
				RT_LATENCY_WITH_CPU_LOAD_SEM;
		break;
	case RT_LATENCY_TEST_CASE_5:
		mask |= RT_LATENCY_WITH_CPU_LOAD |
				RT_LATENCY_WITH_LINUX_LOAD;
		break;
	case RT_LATENCY_TEST_CASE_6:
		mask |= RT_LATENCY_WITH_CPU_LOAD |
				RT_LATENCY_WITH_INVD_CACHE;
		break;
	case RT_LATENCY_TEST_CASE_7:
		/*
		 * TODO: Move/duplicate the code in OCRAM to enable before enabling
		 * RT_LATENCY_TEST_CASE_7
		 */
		mask = -1;
		break;
	default:
		mask = -1;
		break;
	}

	return mask;
}

typedef struct rt_latency_stats {
	struct rtos_apps_stats irq_delay;
	struct rtos_apps_hist irq_delay_hist;

	struct rtos_apps_stats irq_to_sched;
	struct rtos_apps_hist irq_to_sched_hist;

	uint32_t late_alarm_sched;
	bool pending;

} rt_latency_stats_t;

struct rt_latency_ctx {
	os_counter_t *dev;
	os_counter_t *irq_load_dev;

	int tc_load; /* bitmask of the above RT_LATENCY_WITH_xxx test case load conditions */

	rtos_sem_t semaphore; /* used to wake the thread up from IRQ callback */
	rtos_sem_t cpu_load_sem; /* used to increase CPU load through semaphore take/give */
	rtos_sem_t irq_load_sem; /* used to wake the thread up from IRQ load handler */

	uint64_t time_irq;
	uint32_t time_prog;

	rt_latency_stats_t stats;	  /* Current stats tracked by timer task. */
	rt_latency_stats_t stats_snapshot; /* Stats snapshot dump by timer task and printed by logging task. */

	bool quiet;
};

struct ctrl_ctx {
	struct rpmsg_ept *ept;
};

int rt_latency_init(os_counter_t *dev,
		os_counter_t *irq_load_dev, struct rt_latency_ctx *ctx);
int rt_latency_test(struct rt_latency_ctx *ctx);
void rt_latency_destroy(struct rt_latency_ctx *ctx);

void print_stats(struct rt_latency_ctx *ctx);
void cpu_load(struct rt_latency_ctx *ctx);
void cache_inval(void);
void command_handler(void *ctx, struct rpmsg_ept *ept);
int ctrl_ctx_init(struct ctrl_ctx *ctrl);

/* OS specific functions */
int start_test_case(void *context, int test_case_id, bool quiet);
void destroy_test_case(void *context);

#endif /* _RT_LATENCY_H_ */
