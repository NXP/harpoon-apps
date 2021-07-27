/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os.h"
#include "os/stdio.h"

#ifdef OS_ZEPHYR
#include "os_zephyr.h"
#elif defined(OS_FREERTOS)
#include "os_freertos.h"
#endif

#include "rt_latency.h"
#include "rt_tc_setup.h"

extern struct k_thread gpt_thread;

#ifndef SILENT_TESTING
extern struct k_thread print_thread;
#endif

#ifdef ENABLE_HISTOGRAM
#define MAX_STAT_LATENCY	25	/* in us */
static uint32_t isr_latency_record[MAX_STAT_LATENCY];
static uint32_t thread_latency_record[MAX_STAT_LATENCY];
#endif

struct latency_stat rt_stats;

static uint32_t expected_cnt;

#ifdef GPT_DEBUG
static uint32_t start_cycle;
#endif

#ifdef WITH_IRQ_LOAD
static struct k_sem irq_load_sem;
#endif

static inline uint32_t calc_diff_ns(const struct device *dev,
			uint32_t cnt_1, uint32_t cnt_2)
{
	uint32_t diff;
	uint32_t top = os_counter_get_top_value(dev);

	if (os_counter_is_counting_up(dev)) {
		diff =  (cnt_2 < cnt_1) ?
			(cnt_2 + top - cnt_1) : (cnt_2 - cnt_1);
	} else {
		diff = (cnt_2 > cnt_1) ?
			(cnt_1 + top - cnt_2) : (cnt_1 - cnt_2);
	}

	return os_counter_ticks_to_ns(dev, diff);
}


/* Callback function of Counter alarm interrupt handler */
/* irq_counter: the counter value when alarm HW interrupt happens */

static void latency_alarm_handler(const struct device *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
//	uint32_t now;
//	int err;
	uint32_t cur_latency;
	struct latency_stat *rt_stat = user_data;

//	err = os_counter_get_value(dev, &now);
//	os_assert_true(err == 0, "%s: Counter read failed (err: %d)",
//		     dev->name, err);

	cur_latency = calc_diff_ns(dev, expected_cnt, irq_counter);

	rt_stat->act_isr = cur_latency;

	if (rt_stat->cycles < IGNORE_FIRST_CYCLES)
		cur_latency = 0;

	if (rt_stat->max_isr < cur_latency)
		rt_stat->max_isr = cur_latency;

	if (rt_stat->min_isr > cur_latency && rt_stat->cycles >= IGNORE_FIRST_CYCLES)
		rt_stat->min_isr = cur_latency;

	rt_stat->avg_isr += cur_latency;

#ifdef ENABLE_HISTOGRAM
	/* use us in histogram */
	cur_latency /= 1000;
	if (cur_latency >= MAX_STAT_LATENCY)
		cur_latency = MAX_STAT_LATENCY - 1;

	isr_latency_record[cur_latency]++;
#endif

	os_thread_resume(&gpt_thread);
}

#ifdef WITH_IRQ_LOAD

#define IRQ_LOAD_ISR_DURATION_US	10

static void load_alarm_handler(const struct device *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	uint32_t ticks;
	uint32_t start, cur;

	ticks = counter_us_to_ticks(dev, IRQ_LOAD_ISR_DURATION_US);
	os_counter_get_value(dev, &start);
	do {
		os_counter_get_value(dev, &cur);
	} while(cur < start + ticks);

	os_counter_stop(dev);
	os_sem_give(&irq_load_sem);
}
#endif /* WITH_IRQ_LOAD */

static void test_alarm_latency(const struct device *dev,
		const struct device *irq_load_dev, struct latency_stat *rt_stat)
{
	int err;
	uint32_t ticks;
	uint32_t cnt;
	uint32_t counter_period_us;
	struct counter_alarm_cfg alarm_cfg;
#ifdef WITH_IRQ_LOAD
	struct counter_alarm_cfg load_alarm_cfg;
#endif
	uint32_t now;
	uint32_t cur_latency;
#ifdef GPT_DEBUG
	uint32_t cur_cycle;
#endif

	counter_period_us = COUNTER_PERIOD_US_VAL;

	ticks = counter_us_to_ticks(dev, counter_period_us);

	alarm_cfg.flags = COUNTER_ALARM_CFG_ABSOLUTE;
	alarm_cfg.callback = latency_alarm_handler;
	alarm_cfg.user_data = rt_stat;
#ifdef WITH_IRQ_LOAD
	counter_period_us = COUNTER_PERIOD_US_VAL - 1;

	load_alarm_cfg.flags = 0;
	load_alarm_cfg.callback = load_alarm_handler;
	load_alarm_cfg.ticks = counter_us_to_ticks(dev, counter_period_us);

	os_sem_init(&irq_load_sem, 0, 1);
#endif

	do {
		os_counter_start(dev);
#ifdef WITH_IRQ_LOAD
		os_counter_start(irq_load_dev);
		os_sem_reset(&irq_load_sem);
		/* Start irq load alarm firstly */
		err = os_counter_set_channel_alarm(irq_load_dev, 0,
				&load_alarm_cfg);
		os_assert_equal(0, err, "%s: Counter set alarm failed (err: %d)",
			irq_load_dev->name, err);
#endif
		/* Start GPU latency testing alarm */
		os_counter_get_value(dev, &cnt);
		expected_cnt = cnt + ticks;
		alarm_cfg.ticks = expected_cnt;

		err = os_counter_set_channel_alarm(dev, 0, &alarm_cfg);
		os_assert_equal(0, err, "%s: Counter set alarm failed (err: %d)",
			dev->name, err);

		/* Suspend current thread util it is waked up in alarm call back function */
		os_thread_suspend(k_current_get());

		/* Waked up and calculate latency */
		os_counter_get_value(dev, &now);

		cur_latency = calc_diff_ns(dev, expected_cnt, now);

		rt_stat->act_thread_wake = cur_latency;

		if (rt_stat->cycles < IGNORE_FIRST_CYCLES)
			cur_latency = 0;

		if (rt_stat->max_thread_wake < cur_latency)
			rt_stat->max_thread_wake = cur_latency;

		if (rt_stat->min_thread_wake > cur_latency && rt_stat->cycles >= IGNORE_FIRST_CYCLES)
			rt_stat->min_thread_wake = cur_latency;

		rt_stat->avg_thread_wake += cur_latency;

#ifdef ENABLE_HISTOGRAM
		/* use us in histogram */
		cur_latency /= 1000;
		if (cur_latency >= MAX_STAT_LATENCY)
			cur_latency = MAX_STAT_LATENCY - 1;
		thread_latency_record[cur_latency]++;
#endif
		rt_stat->cycles++;
		os_counter_stop(dev);

#ifndef SILENT_TESTING
		os_thread_resume(&print_thread);
#endif

#ifdef	WITH_IRQ_LOAD
		/* Waiting irq load ISR exits and then go to next loop */
		os_sem_take(&irq_load_sem, K_FOREVER);
#endif
	} while(rt_stat->cycles < TESTING_LOOP_NUM);

}

void gpt_latency_test(void *p1, void *p2, void *p3)
{
	const struct device *dev = p1;
	struct latency_stat *rt_stat = p2;
	const struct device *irq_load_dev = p3;

	rt_stat->min_isr = 0xffffffff;
	rt_stat->min_thread_wake = 0xffffffff;

#ifdef OS_ZEPHYR
	k_object_access_grant(dev, k_current_get());
#endif
	test_alarm_latency(dev, irq_load_dev, rt_stat);

	os_usleep(2000);
	os_thread_abort(k_current_get());
}

/* CPU Load */
#ifdef WITH_CPU_LOAD

#define CPU_LOAD_SWITH_CYCLES 0x8fffffff

void cpu_load(void *p1, void *p2, void *p3)
{
	struct k_sem cpu_sem;

	os_sem_init(&cpu_sem, 0, UINT_MAX);

	do {
#ifdef WITH_CPU_LOAD_SEM
		os_sem_give(&cpu_sem);
		os_sem_take(&cpu_sem, K_FOREVER);
#endif
#ifdef WITH_INVD_CACHE
		os_invd_dcache_all();
		os_invd_icache_all();
		/* 10Hz */
		k_busy_wait(USEC_PER_MSEC * 100U);
#endif
	} while(1);
}
#endif

static inline void print_cur_stat(void)
{
#ifdef GPT_DEBUG
	printf("C:%7lu, Min:%7d-%d, Act:%8d-%d, Avg:%8ld-%ld, Max:%8d-%d, %u-%u\n",
#else
	printf("C:%7lu, Min:%7d-%d, Act:%8d-%d, Avg:%8ld-%ld, Max:%8d-%d\n",
#endif
		rt_stats.cycles,
		rt_stats.min_isr,
		rt_stats.min_thread_wake,
		rt_stats.act_isr,
		rt_stats.act_thread_wake,
		rt_stats.cycles > IGNORE_FIRST_CYCLES ?
			(long) rt_stats.avg_isr /
			(rt_stats.cycles - IGNORE_FIRST_CYCLES) : 0,
		rt_stats.cycles > IGNORE_FIRST_CYCLES ?
			(long) rt_stats.avg_thread_wake /
			(rt_stats.cycles - IGNORE_FIRST_CYCLES) : 0,
		rt_stats.max_isr,
#ifdef GPT_DEBUG
		rt_stats.max_thread_wake,
		rt_stats.counter_diff,
		rt_stats.cycle_diff);
#else
		rt_stats.max_thread_wake);
#endif

}

void print_stats(struct latency_stat *rt_stat)
{
	unsigned long pre_cycles = 0;

	do {
		if (rt_stats.cycles > pre_cycles) {
			print_cur_stat();
			pre_cycles = rt_stats.cycles;
		}

		//k_usleep(2000);
		os_thread_suspend(k_current_get());

	} while(1);
}

#ifdef ENABLE_HISTOGRAM
void print_records(void)
{
	int j;

	os_printf("\nLatency Record Lists:\n\t\t");
	os_printf("      GPT STATs\t");
	os_printf("\nLatency(us)\t    ");
	os_printf("(isr - task)\t    ");

	os_printf("\n");
	for (j = 0; j < MAX_STAT_LATENCY; j++) {
		os_printf("\t%02d", j);
		os_printf("\t%08d - %08d", isr_latency_record[j],
					thread_latency_record[j]);
		os_printf("\n");
	}
}
#endif

void print_summary(void)
{
	os_printf("\nTesting Setup:\n");
#ifdef CONFIG_SMP
	os_printf("\tUsing the following threads on SMP kernel with %d CPU Cores:\n",
			CONFIG_MP_NUM_CPUS);
#else
	os_printf("\tUsing the following threads on non-SMP kernel:\n");
#endif

	os_printf("\t\tGPT testing threads with one GPT Counter irq.\n");

#ifndef SILENT_TESTING
	os_printf("\t\tOne realtime printing thread.\n");
#endif

#ifdef WITH_CPU_LOAD
#ifdef WITH_CPU_LOAD_SEM
	os_printf("\t\tOne CPU load thread with Semaphore load.\n");
#else
	os_printf("\t\tOne CPU load thread.\n");
#endif
#endif

#ifdef WITH_IRQ_LOAD
	os_printf("\t\tOne IRQ load and thread.\n");
#endif

#ifdef THREAD_CPU_BINDING
	os_printf("\tUsing CPU binding:\n");
	os_printf("\t\tGPT_thread on Core%d, Load_thread on Core%d",
			GPT_CPU_BINDING,
			PRINT_CPU_BINDING,
			LOAD_CPU_BINDING);
#ifndef SILENT_TESTING
	os_printf("\t\t, Print_thread on Core%d",
			PRINT_CPU_BINDING);
#endif
	os_printf(".\n");
#endif

#ifdef ENABLE_HISTOGRAM
	print_records();
#endif

	os_printf("\nTesting Result:\n");
	os_printf("\t");
	print_cur_stat();
}

