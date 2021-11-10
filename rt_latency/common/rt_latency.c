/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cpu.h"

#include "os.h"
#include "os/assert.h"
#include "os/counter.h"
#include "os/semaphore.h"
#include "os/stdio.h"
#include "os/unistd.h"

#include "stats.h"

#include "rt_latency.h"
#include "rt_tc_setup.h"

static inline uint32_t calc_diff_ns(const void *dev,
			uint32_t cnt_1, uint32_t cnt_2)
{
	uint32_t diff;
	static uint32_t top = 0;
	static int counting_up = -1;

	/*
	 * Set these variables once for all to avoid multiple register accesses
	 * each time we call this function.
	 */
	if (top == 0)
		top = os_counter_get_top_value(dev);

	if (counting_up == -1)
		counting_up = os_counter_is_counting_up(dev);

	if (counting_up) {
		diff =  (cnt_2 < cnt_1) ?
			(cnt_2 + top - cnt_1) : (cnt_2 - cnt_1);
	} else {
		diff = (cnt_2 > cnt_1) ?
			(cnt_1 + top - cnt_2) : (cnt_1 - cnt_2);
	}

	return os_counter_ticks_to_ns(dev, diff);
}

/*
 * Used by all RTOS:
 *    o FreeRTOS through IRQ_Handler_GPT() handler
 *    o Zephyr through the alarm's ->callback
 *
 * @current_counter: counter value when the IRQ occurred
 */
static void latency_alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	struct latency_stat *rt_stat = user_data;

	rt_stat->time_irq = irq_counter;

	os_sem_give(&rt_stat->semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

#ifdef WITH_IRQ_LOAD

#define IRQ_LOAD_ISR_DURATION_US	10

static void load_alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	uint32_t start, cur;
	struct latency_stat *rt_stat = user_data;

	os_counter_get_value(dev, &start);

	do {
		os_counter_get_value(dev, &cur);
	} while (calc_diff_ns(dev, start, cur) < IRQ_LOAD_ISR_DURATION_US * 1000);

	os_counter_stop(dev);
	os_sem_give(&rt_stat->irq_load_sem, OS_SEM_FLAGS_ISR_CONTEXT);
}
#endif /* WITH_IRQ_LOAD */

/*
 * Blocking function including an infinite loop ;
 * must be called by separate threads/tasks.
 *
 * In case an error is returned the caller shall handle the lifecycle of the
 * thread by suspending or destroying the latter.
 */
int rt_latency_test(struct latency_stat *rt_stat)
{
	int err;
	uint32_t cnt, ticks;
	uint32_t counter_period_us;
	struct os_counter_alarm_cfg alarm_cfg;
	uint32_t now;
	uint64_t irq_delay;
	uint64_t irq_to_sched;
	const void *dev = rt_stat->dev;
#ifdef WITH_IRQ_LOAD
	struct os_counter_alarm_cfg load_alarm_cfg;
	const void *irq_load_dev = rt_stat->irq_load_dev;
#endif

	counter_period_us = COUNTER_PERIOD_US_VAL;
	ticks = os_counter_us_to_ticks(dev, counter_period_us);

	alarm_cfg.flags = OS_COUNTER_ALARM_CFG_ABSOLUTE;
	alarm_cfg.user_data = rt_stat;
	alarm_cfg.callback = latency_alarm_handler;
#ifdef WITH_IRQ_LOAD
	counter_period_us = COUNTER_PERIOD_US_VAL - 1;

	load_alarm_cfg.flags = 0;
	load_alarm_cfg.user_data = rt_stat;
	load_alarm_cfg.callback = load_alarm_handler;
	load_alarm_cfg.ticks = os_counter_us_to_ticks(dev, counter_period_us);

	os_sem_init(&rt_stat->irq_load_sem, 0);
#endif

	do {
		os_counter_start(dev);
#ifdef WITH_IRQ_LOAD
		os_counter_start(irq_load_dev);
		/* Start irq load alarm firstly */
		err = os_counter_set_channel_alarm(irq_load_dev, 0,
				&load_alarm_cfg);
		os_assert(!err, "Counter set alarm failed (err: %d)", err);
#endif
		/* Start IRQ latency testing alarm */
		os_counter_get_value(dev, &cnt);
		alarm_cfg.ticks = cnt + ticks;

		rt_stat->time_prog = alarm_cfg.ticks;

		err = os_counter_set_channel_alarm(dev, 0, &alarm_cfg);
		os_assert(!err, "Counter set alarm failed (err: %d)", err);

		/* Sync current thread with alarm callback function thanks to a semaphore */
		err = os_sem_take(&rt_stat->semaphore, 0, OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the semaphore (err: %d)", err);

		/* Woken up... calculate latency */
		os_counter_get_value(dev, &now);

		irq_delay = calc_diff_ns(dev, rt_stat->time_prog, rt_stat->time_irq);

		stats_update(&rt_stat->irq_delay, irq_delay);
		hist_update(&rt_stat->irq_delay_hist, irq_delay);

		irq_to_sched = calc_diff_ns(dev, rt_stat->time_prog, now);
		stats_update(&rt_stat->irq_to_sched, irq_to_sched);
		hist_update(&rt_stat->irq_to_sched_hist, irq_to_sched);

		os_counter_stop(dev);

#ifdef	WITH_IRQ_LOAD
		/* Waiting irq load ISR exits and then go to next loop */
		err = os_sem_take(&rt_stat->irq_load_sem, 0, OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the semaphore (err: %d)", err);
#endif
	} while(1);
}

/* CPU Load */
#ifdef WITH_CPU_LOAD

void cpu_load(void)
{
#ifdef WITH_CPU_LOAD_SEM
	int err;
	os_sem_t cpu_sem;

	err = os_sem_init(&cpu_sem, 0);
	os_assert(!err, "semaphore init failed!");
#endif
	os_printf("%s: task started\n\r", __func__);

	do {
#ifdef WITH_CPU_LOAD_SEM
		err = os_sem_take(&cpu_sem, 0, OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Failed to take semaphore");

		err = os_sem_give(&cpu_sem, 0);
		os_assert(!err, "Failed to give semaphore");
#endif
	} while(1);
}
#endif /* #ifdef WITH_CPU_LOAD */

#ifdef WITH_INVD_CACHE
#define CACHE_INVAL_PERIOD_MS (100)
void cache_inval(void)
{
	os_printf("%s: task started\n\r", __func__);

	do {
		os_invd_dcache_all();
		os_invd_icache_all();

		os_msleep(CACHE_INVAL_PERIOD_MS);

	} while(1);
}
#endif /* #ifdef WITH_INVD_CACHE */

void print_stats(struct latency_stat *rt_stat)
{
	do {
		os_msleep(10000);

		stats_compute(&rt_stat->irq_delay);
		stats_print(&rt_stat->irq_delay);
		stats_reset(&rt_stat->irq_delay);
		hist_print(&rt_stat->irq_delay_hist);

		stats_compute(&rt_stat->irq_to_sched);
		stats_print(&rt_stat->irq_to_sched);
		stats_reset(&rt_stat->irq_to_sched);
		hist_print(&rt_stat->irq_to_sched_hist);

		os_printf("\n\r");

	} while(1);
}

int rt_latency_init(const void *dev,
		const void *irq_load_dev, struct latency_stat *rt_stat)
{
	int err;

	rt_stat->dev = dev;
	rt_stat->irq_load_dev = irq_load_dev;

	stats_init(&rt_stat->irq_delay, 31, "irq delay (ns)", NULL);
	hist_init(&rt_stat->irq_delay_hist, 20, 200);

	stats_init(&rt_stat->irq_to_sched, 31, "irq to sched (ns)", NULL);
	hist_init(&rt_stat->irq_to_sched_hist, 20, 1000);

	err = os_sem_init(&rt_stat->semaphore, 0);
	os_assert(!err, "semaphore creation failed!");

	return err;
}

