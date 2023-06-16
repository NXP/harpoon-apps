/*
 * Copyright 2022-2023 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <string.h>

#include "os/assert.h"

#include "hlog.h"
#include "rt_latency.h"

#define STACK_SIZE 4096

K_THREAD_STACK_DEFINE(counter_stack, STACK_SIZE);

K_THREAD_STACK_DEFINE(cpu_load_stack, STACK_SIZE);

K_THREAD_STACK_DEFINE(cache_inval_stack, STACK_SIZE);

#ifndef SILENT_TESTING
K_THREAD_STACK_DEFINE(print_stack, STACK_SIZE);
#endif

#define MAX_TC_THREADS	8

static struct main_ctx{
	int test_case_id;
	bool started;
	struct rt_latency_ctx rt_ctx;
	struct ctrl_ctx ctrl;
	struct k_thread tc_thread[MAX_TC_THREADS];
	int threads_running_count;
} main_ctx;


static void counter_latency_test(void *p1, void *p2, void *p3)
{
	struct rt_latency_ctx *ctx = p1;
	const struct device *dev = (const struct device *)ctx->dev;
	int ret;

	k_object_access_grant(dev, k_current_get());

	do {
		ret = rt_latency_test(ctx);
		if (ret)
		{
			log_err("test failed!\n");
			k_thread_suspend(k_current_get());
		}
	} while (!ret);

	k_usleep(2000);
	k_thread_abort(k_current_get());
}

static void cpu_load_func(void *p1, void *p2, void *p3)
{
	struct rt_latency_ctx *ctx = p1;

	log_info("running cpu load %s\r\n",
		ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD_SEM ? " (with extra semaphore load)" : "");

	do {
		cpu_load(ctx);
	} while(1);
}

static void cache_inval_func(void *p1, void *p2, void *p3)
{
	log_info("running cache invalidation\r\n");

	do {
		cache_inval();
	} while(1);
}

static void print_stats_func(void *p1, void *p2, void *p3)
{
	struct rt_latency_ctx *ctx = p1;

	do {
		k_msleep(STATS_PERIOD_SEC * 1000);

		print_stats(ctx);
	} while(1);
}

int start_test_case(void *context, int test_case_id)
{
	struct main_ctx *ctx = context;
	const struct device *counter_dev;
	const struct device *irq_load_dev;
	struct k_thread *benchmark_thread;
	struct k_thread *cpu_load_thread;
	struct k_thread *cache_invld_thread;
#ifndef SILENT_TESTING
	struct k_thread *print_thread;
#endif
	int ret;

	if (ctx->started)
		return -1;

	log_info("Running test case %d:\n", test_case_id);

	ctx->test_case_id = test_case_id;

	/* Initialize test case load conditions based on test case ID */
	ctx->rt_ctx.tc_load = rt_latency_get_tc_load(test_case_id);
	if (ctx->rt_ctx.tc_load < 0) {
		log_err("Wrong test conditions!\n");
		goto err;
	}

	/* Give required clocks some time to stabilize. In particular, nRF SoCs
	 * need such delay for the Xtal LF clock source to start and for this
	 * test to use the correct timing.
	 */
	k_busy_wait(USEC_PER_MSEC * 300);

	/* Create GPT threads with Highest Priority*/
	counter_dev = DEVICE_DT_GET(DT_ALIAS(counter0));
	if (!counter_dev) {
		log_err("Unable to get main counter device\n");
		goto err;
	}

	/* Use the second counter instance to create irq load with lower priority */
	irq_load_dev = DEVICE_DT_GET(DT_ALIAS(counter1));
	if (!irq_load_dev) {
		log_err("Unable to get IRQ load counter device\n");
		goto err;
	}

	/* Initialize test cases' context */
	ret = rt_latency_init((os_counter_t *)counter_dev, (os_counter_t *)irq_load_dev, &ctx->rt_ctx);
	if (ret != 0) {
		log_err("Initialization failed!\n");
		goto err;
	}

	benchmark_thread = &ctx->tc_thread[ctx->threads_running_count++];
	/* Benchmark task: main "high prio IRQ" task */
	k_thread_create(benchmark_thread, counter_stack, STACK_SIZE,
		counter_latency_test, &ctx->rt_ctx, NULL, NULL,
		K_HIGHEST_THREAD_PRIO, 0, K_FOREVER);

	k_busy_wait(USEC_PER_MSEC * 300);
#ifdef THREAD_CPU_BINDING
	k_thread_cpu_mask_clear(benchmark_thread);
	k_thread_cpu_mask_enable(benchmark_thread, GPT_CPU_BINDING);
#endif

#ifndef SILENT_TESTING
	/* Print Thread */
	print_thread = &ctx->tc_thread[ctx->threads_running_count++];
	k_thread_create(print_thread, print_stack, STACK_SIZE,
			print_stats_func, &ctx->rt_ctx, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO - 2, 0, K_FOREVER);
#ifdef THREAD_CPU_BINDING
	k_thread_cpu_mask_clear(print_thread);
	k_thread_cpu_mask_enable(print_thread, PRINT_CPU_BINDING);
#endif
	k_thread_start(print_thread);
#endif

	/* CPU Load Thread */
	if (ctx->rt_ctx.tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		cpu_load_thread = &ctx->tc_thread[ctx->threads_running_count++];
		k_thread_create(cpu_load_thread, cpu_load_stack, STACK_SIZE,
			cpu_load_func, &ctx->rt_ctx, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_FOREVER);
#ifdef THREAD_CPU_BINDING
		k_thread_cpu_mask_clear(cpu_load_thread);
		k_thread_cpu_mask_enable(cpu_load_thread, CPU_LOAD_CPU_BINDING);
#endif
		k_thread_start(cpu_load_thread);
	}

	/* Cache Invalidate Thread */
	if (ctx->rt_ctx.tc_load & RT_LATENCY_WITH_INVD_CACHE) {
		cache_invld_thread = &ctx->tc_thread[ctx->threads_running_count++];
		k_thread_create(cache_invld_thread, cache_inval_stack, STACK_SIZE,
			cache_inval_func, NULL, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO - 1, 0, K_FOREVER);
#ifdef THREAD_CPU_BINDING
		k_thread_cpu_mask_clear(cache_invld_thread);
		k_thread_cpu_mask_enable(cache_invld_thread, CPU_LOAD_CPU_BINDING);
#endif
		k_thread_start(cache_invld_thread);
	}

	/* Start GPT Threads */
	k_thread_start(benchmark_thread);

	ctx->started = true;

	return 0;
err:
	destroy_test_case(ctx);

	return -1;
}

void destroy_test_case(void *context)
{
	struct main_ctx *ctx = context;
	int i;

	if (!ctx->started)
		return;

	for (i = 0; i < ctx->threads_running_count; i++) {
		k_thread_abort(&ctx->tc_thread[i]);
	}

	ctx->threads_running_count = 0;

	rt_latency_destroy(&ctx->rt_ctx);

	ctx->started = false;
}

void main(void)
{
	struct main_ctx *ctx = &main_ctx;
	int rc;

	log_info("running\n");

	memset(ctx, 0, sizeof(*ctx));

	ctx->started = false;

	rc = ctrl_ctx_init(&ctx->ctrl);
	os_assert(!rc, "ctrl context failed!");

	do {
		command_handler(ctx, ctx->ctrl.ept);

		k_msleep(100);

	} while(1);
}
