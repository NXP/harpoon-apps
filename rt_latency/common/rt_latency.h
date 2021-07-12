/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RT_LATENCY_H_
#define _RT_LATENCY_H_

struct latency_stat {
	unsigned long cycles;
	uint32_t min_isr;
	uint32_t max_isr;
	uint32_t act_isr;
	double avg_isr;
	uint32_t min_thread_wake;
	uint32_t max_thread_wake;
	uint32_t act_thread_wake;
	double avg_thread_wake;
	uint32_t max_total;
#ifdef GPT_DEBUG
	uint32_t counter_diff;
	uint32_t cycle_diff;
#endif
};

void gpt_latency_test(void *p1, void *p2, void *p3);

void cpu_load(void *p1, void *p2, void *p3);

void irq_load(void *p1, void *p2, void *p3);

void print_stats(void *p1, void *p2, void *p3);

void print_summary(void);

#endif /* _RT_LATENCY_H_ */
