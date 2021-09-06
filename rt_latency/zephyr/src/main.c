/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ztest.h>
#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <sys/printk.h>

#include "rt_latency.h"
#include "rt_tc_setup.h"

struct k_thread gpt_thread;

#ifdef WITH_CPU_LOAD
struct k_thread cpu_load_thread;
#endif
#ifndef SILENT_TESTING
struct k_thread print_thread;
#endif

#define STACK_SIZE (640 + CONFIG_TEST_EXTRA_STACKSIZE)

K_THREAD_STACK_DEFINE(gpt_stack, STACK_SIZE);

#ifdef WITH_CPU_LOAD
K_THREAD_STACK_DEFINE(cpu_load_stack, STACK_SIZE);
#endif

#ifndef SILENT_TESTING
K_THREAD_STACK_DEFINE(print_stack, STACK_SIZE);
#endif

#define INST_DT_COMPAT_LABEL(n, compat) DT_LABEL(DT_INST(n, compat)),
/* Generate a list of LABELs for all instances of the "compat" */
#define LABELS_FOR_DT_COMPAT(compat) \
	COND_CODE_1(DT_HAS_COMPAT_STATUS_OKAY(compat), \
		   (UTIL_LISTIFY(DT_NUM_INST_STATUS_OKAY(compat), \
				 INST_DT_COMPAT_LABEL, compat)), ())

static const char * const devices[] = {
	LABELS_FOR_DT_COMPAT(nxp_imx_gpt)
};

extern struct latency_stat rt_stats;

void print_stats_func(void *p1, void *p2, void *p3)
{
	/* TODO : pass latency_stat object */
	print_stats(NULL);
}

void test_main(void)
{
	const struct device *gpt_dev;
#ifdef WITH_IRQ_LOAD
	const struct device *irq_load_dev;
#endif
	void *p3 = NULL;

	/* Give required clocks some time to stabilize. In particular, nRF SoCs
	 * need such delay for the Xtal LF clock source to start and for this
	 * test to use the correct timing.
	 */
	k_busy_wait(USEC_PER_MSEC * 300);

	/* Create GPT threads with Highest Priority*/
	gpt_dev = device_get_binding(devices[0]);
	if(!gpt_dev)
		printk("Unable to get counter device\n");

#ifdef WITH_IRQ_LOAD
	/* Use the second GPU Counter to create irq load with lower priority */
	irq_load_dev = device_get_binding(devices[1]);
	if(!irq_load_dev)
		printk("Unable to get counter device\n");
	p3 = (void *)irq_load_dev;
#endif
	k_thread_create(&gpt_thread, gpt_stack, STACK_SIZE,
		gpt_latency_test, (void *)gpt_dev, &rt_stats, p3,
		K_HIGHEST_THREAD_PRIO, 0, K_FOREVER);

	k_busy_wait(USEC_PER_MSEC * 300);
#ifdef THREAD_CPU_BINDING
	k_thread_cpu_mask_clear(&gpt_thread);
	k_thread_cpu_mask_enable(&gpt_thread, GPT_CPU_BINDING);
#endif

#ifndef SILENT_TESTING
	/* Print Thread */
	k_thread_create(&print_thread, print_stack, STACK_SIZE,
			print_stats_func, NULL, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO - 1, 0, K_FOREVER);
#ifdef THREAD_CPU_BINDING
	k_thread_cpu_mask_clear(&print_thread);
	k_thread_cpu_mask_enable(&print_thread, PRINT_CPU_BINDING);
#endif
	k_thread_start(&print_thread);
#endif

	/* CPU Load Thread */
#ifdef WITH_CPU_LOAD
	k_thread_create(&cpu_load_thread, cpu_load_stack, STACK_SIZE,
			cpu_load, NULL, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_FOREVER);
#ifdef THREAD_CPU_BINDING
	k_thread_cpu_mask_clear(&cpu_load_thread);
	k_thread_cpu_mask_enable(&cpu_load_thread, CPU_LOAD_CPU_BINDING);
#endif
	k_thread_start(&cpu_load_thread);
#endif

	/* Start GPT Threads */
	k_thread_start(&gpt_thread);

	k_thread_join(&gpt_thread, K_FOREVER);

#ifndef SILENT_TESTING
	k_thread_abort(&print_thread);
#endif

	print_summary();
}
