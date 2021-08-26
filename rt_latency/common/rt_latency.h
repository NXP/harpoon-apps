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

struct latency_stat {
	const void *dev;
	const void *irq_load_dev;

	os_sem_t semaphore; /* used to wake the thread up from IRQ callback */
#ifdef WITH_IRQ_LOAD
	os_sem_t irq_load_sem; /* used to wake the thread up from IRQ load handler */
#endif

	uint64_t time_irq;
	uint32_t time_prog;

	struct stats irq_delay;
	struct hist irq_delay_hist;

	struct stats irq_to_sched;
	struct hist irq_to_sched_hist;
};

int rt_latency_init(const void *dev,
		const void *irq_load_dev, struct latency_stat *rt_stat);

int rt_latency_test(struct latency_stat *rt_stat);

void print_stats(struct latency_stat *rt_stat);
#ifdef WITH_CPU_LOAD
void cpu_load(void);
#endif
#ifdef WITH_IRQ_LOAD
void irq_load(void *p1, void *p2, void *p3);
/* TODO: Register these handlers as alarm callback function */
void load_alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter, void *user_data);
#endif
void latency_alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter, void *user_data);

#endif /* _RT_LATENCY_H_ */
