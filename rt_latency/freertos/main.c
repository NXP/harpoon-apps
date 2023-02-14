/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* Freescale includes. */
#include "board.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_gpt.h"

/* Harpoon-apps includes. */
#include "os/assert.h"
#include "os/counter.h"
#include "os/semaphore.h"

#include "stats.h"
#include "ivshmem.h"
#include "hlog.h"
#include "mailbox.h"
#include "version.h"

#ifdef MBOX_TRANSPORT_RPMSG
#include "rpmsg.h"
#endif

#include "rt_latency.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SOURCE_CLOCK_FREQ_MHZ	24
#define NUM_OF_COUNTER		2

/* Task priorities. */
#define HIGHEST_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define LOWEST_TASK_PRIORITY  (0)

#define STACK_SIZE (configMINIMAL_STACK_SIZE + 100)
#define MAIN_STACK_SIZE	(STACK_SIZE + 1024)

#ifdef MBOX_TRANSPORT_RPMSG
#define EPT_ADDR (30)
#endif

/*******************************************************************************
 * Globals
 ******************************************************************************/

GPT_Type *gpt_devices[NUM_OF_COUNTER] = {GPT1, GPT2};

static struct main_ctx{
	bool started;

	struct rt_latency_ctx rt_ctx;

	/* hard-coded number of elements ; only used to create/delete test case's
	* task handles, all at once */
	TaskHandle_t tc_taskHandles[8];
} main_ctx;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Application tasks */
void main_task(void *pvParameters);
void benchmark_task(void *pvParameters);
void log_task(void *pvParameters);
void cpu_load_task(void *pvParameters);
void cache_inval_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/

void cpu_load_task(void *pvParameters)
{
	struct rt_latency_ctx *ctx = pvParameters;

	log_info("running%s\n", ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD_SEM ?
		       " (with extra semaphore load)" : "");

	do {
		cpu_load(ctx);
	} while(1);
}

void cache_inval_task(void *pvParameters)
{
	log_info("running\n");

	do {
		cache_inval();
	} while(1);
}

void log_task(void *pvParameters)
{
	struct rt_latency_ctx *ctx = pvParameters;

	do {
		vTaskDelay(pdMS_TO_TICKS(STATS_PERIOD_SEC * 1000));

		print_stats(ctx);
	} while(1);
}

void benchmark_task(void *pvParameters)
{
	int ret;
	struct rt_latency_ctx *ctx = pvParameters;

	log_info("running%s\n",
	       (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD)  ? " (with IRQ load)" : "");

	do {
		ret = rt_latency_test(ctx);
		if (ret)
		{
			log_err("test failed!\n");
			vTaskSuspend(NULL);
		}
	} while (!ret);
}

/*******************************************************************************
 * Application functions
 ******************************************************************************/

void destroy_test_case(void *context)
{
	struct main_ctx *ctx = context;
	int hnd_idx;

	if (!ctx->started)
		return;

	for (hnd_idx = 0; hnd_idx < ARRAY_SIZE(ctx->tc_taskHandles); hnd_idx++) {
		if (ctx->tc_taskHandles[hnd_idx]) {
			vTaskDelete(ctx->tc_taskHandles[hnd_idx]);
			ctx->tc_taskHandles[hnd_idx] = NULL;
		}
	}

	rt_latency_destroy(&ctx->rt_ctx);

	ctx->started = false;
}

int start_test_case(void *context, int test_case_id)
{
	struct main_ctx *ctx = context;
	void *dev;
	void *irq_load_dev = NULL;
	int hnd_idx = 0;
	BaseType_t xResult;

	if (ctx->started)
		return -1;

	log(INFO, "---\n");
	log_info("Running test case %d:\n", test_case_id);

	dev = gpt_devices[0]; /* GPT1 */
	irq_load_dev = gpt_devices[1]; /* GPT2 */

	/* Initialize test case load conditions based on test case ID */
	ctx->rt_ctx.tc_load = rt_latency_get_tc_load(test_case_id);
	if (ctx->rt_ctx.tc_load < 0) {
		log_err("Wrong test conditions!\n");
		goto err;
	}

	/* Initialize test cases' context */
	xResult = rt_latency_init(dev, irq_load_dev, &ctx->rt_ctx);
	if (xResult) {
		log_err("Initialization failed!\n");
		goto err;
	}

	/* Benchmark task: main "high prio IRQ" task */
	xResult = xTaskCreate(benchmark_task, "benchmark_task", STACK_SIZE,
			       &ctx->rt_ctx, HIGHEST_TASK_PRIORITY - 1, &ctx->tc_taskHandles[hnd_idx++]);
	if (xResult != pdPASS) {
		log_err("task creation failed!\n");
		goto err_task;
	}

	/* CPU Load task */
	if (ctx->rt_ctx.tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		xResult = xTaskCreate(cpu_load_task, "cpu_load_task", STACK_SIZE,
				       &ctx->rt_ctx, LOWEST_TASK_PRIORITY, &ctx->tc_taskHandles[hnd_idx++]);
		if (xResult != pdPASS) {
			log_err("task creation failed!\n");
			goto err_task;
		}
	}

	/* Cache invalidate task */
	if (ctx->rt_ctx.tc_load & RT_LATENCY_WITH_INVD_CACHE) {
		xResult = xTaskCreate(cache_inval_task, "cache_inval_task",
			       STACK_SIZE, NULL, LOWEST_TASK_PRIORITY + 1, &ctx->tc_taskHandles[hnd_idx++]);
		if (xResult != pdPASS) {
			log_err("task creation failed!\n");
			goto err_task;
		}
	}

	/* Print task */
	xResult = xTaskCreate(log_task, "log_task", STACK_SIZE,
				&ctx->rt_ctx, LOWEST_TASK_PRIORITY + 1, &ctx->tc_taskHandles[hnd_idx++]);
	if (xResult != pdPASS) {
		log_err("task creation failed!\n");
		goto err_task;
	}

	ctx->started = true;

	return 0;

err_task:
	/* hnd_idx is being incremented before the task creation */
	for (hnd_idx -= 2; hnd_idx >= 0; hnd_idx--) {
		if (ctx->tc_taskHandles[hnd_idx]) {
			vTaskDelete(ctx->tc_taskHandles[hnd_idx]);
			ctx->tc_taskHandles[hnd_idx] = NULL;
		}
	}

	rt_latency_destroy(&ctx->rt_ctx);

err:
	return -1;
}

void main_task(void *pvParameters)
{
	struct main_ctx *ctx = pvParameters;
	struct mailbox m;
	void *cmd, *resp;
	void *tp = NULL;
	int rc;

	log_info("Harpoon v%s\n", VERSION);

	log_info("running\n");

#ifdef MBOX_TRANSPORT_RPMSG
	rc = rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw",
				  &tp, &cmd, &resp);
	os_assert(!rc, "rpmsg transport initialization failed, cannot proceed\n");
#else /* IVSHMEM */
	rc = ivshmem_transport_init(0, NULL, &tp, &cmd, &resp);
	os_assert(!rc, "ivshmem transport initialization failed, cannot proceed\n");
#endif

	mailbox_init(&m, cmd, resp, false, tp);

	ctx->started = false;

	do {
		command_handler(ctx, &m);

		vTaskDelay(pdMS_TO_TICKS(100));

	} while(1);
}

/*!
 * @brief Main function
 */
int main(void)
{
	BaseType_t xResult;

	/* Init board cpu and hardware. */
	BOARD_InitMemory();
	BOARD_InitDebugConsole();

	/* Test cases scheduler task */
	xResult = xTaskCreate(main_task, "main_task", MAIN_STACK_SIZE,
			&main_ctx, LOWEST_TASK_PRIORITY + 1, NULL);
	assert(xResult == pdPASS);

	/* Start scheduler */
	vTaskStartScheduler();

	for (;;)
		;


	return xResult;
}

