/*
 * Copyright 2021-2024 NXP
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

/* Harpoon-apps includes. */
#include "os/counter.h"
#include "os/semaphore.h"

#include "stats.h"
#include "hlog.h"
#include "version.h"
#include "rtos_abstraction_layer.h"

#include "rt_latency.h"
#if __has_include("clock_config.h")
#include "clock_config.h"
#endif


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

#define EPT_ADDR (30)


/*******************************************************************************
 * Globals
 ******************************************************************************/

static struct main_ctx{
	bool started;

	struct rt_latency_ctx rt_ctx;
	struct ctrl_ctx ctrl;

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
__WEAK void BOARD_InitClocks(void) {}

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
	os_counter_t *main_counter_dev;
	os_counter_t *irq_load_dev;
	int hnd_idx = 0;
	BaseType_t xResult;

	if (ctx->started)
		return -1;

	log(INFO, "---\n");
	log_info("Running test case %d:\n", test_case_id);

	main_counter_dev = GET_COUNTER_DEVICE_INSTANCE(0); //GPT1
	irq_load_dev = GET_COUNTER_DEVICE_INSTANCE(1); //GPT2

	/* Initialize test case load conditions based on test case ID */
	ctx->rt_ctx.tc_load = rt_latency_get_tc_load(test_case_id);
	if (ctx->rt_ctx.tc_load < 0) {
		log_err("Wrong test conditions!\n");
		goto err;
	}

	/* Initialize test cases' context */
	xResult = rt_latency_init(main_counter_dev, irq_load_dev, &ctx->rt_ctx);
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
	int rc;

	log_info("Harpoon v%s\n", VERSION);

	log_info("running\n");

	ctx->started = false;

	rc = ctrl_ctx_init(&ctx->ctrl);
	rtos_assert(!rc, "ctrl context failed!");

	do {
		command_handler(ctx, ctx->ctrl.ept);

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
	BOARD_InitClocks();
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

