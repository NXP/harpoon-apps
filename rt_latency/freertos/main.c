/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* System includes.*/
#include <stdio.h>

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
#include "os.h"
#include "os/counter.h"
#include "os/semaphore.h"
#include "os/stdio.h"

#include "stats.h"
#include "version.h"

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

/*******************************************************************************
 * Globals
 ******************************************************************************/

GPT_Type *gpt_devices[NUM_OF_COUNTER] = {GPT1, GPT2};

static struct rt_latency_ctx rt_ctx;

/* hard-coded number of elements ; only used to create/delete test case's
 * task handles, all at once */
static TaskHandle_t tc_taskHandles[8];

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

	cpu_load(ctx);
}

void cache_inval_task(void *pvParameters)
{
	cache_inval();
}

void log_task(void *pvParameters)
{
	struct rt_latency_ctx *ctx = pvParameters;

	print_stats(ctx);
}

void benchmark_task(void *pvParameters)
{
	int ret;
	struct rt_latency_ctx *ctx = pvParameters;

	os_printf("%s: running%s\n\r", __func__,
	       (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD)  ? " (with IRQ load)" : "");

	ret = rt_latency_test(ctx);
	if (ret)
	{
		os_printf("test failed!\n");
		vTaskSuspend(NULL);
	}
}

/*******************************************************************************
 * Application functions
 ******************************************************************************/

static void destroy_test_case(void)
{
	int hnd_idx;

	for (hnd_idx = 0; hnd_idx < ARRAY_SIZE(tc_taskHandles); hnd_idx++) {
		if (tc_taskHandles[hnd_idx]) {
			vTaskDelete(tc_taskHandles[hnd_idx]);
			tc_taskHandles[hnd_idx] = NULL;
		}
	}

	rt_latency_destroy(&rt_ctx);
}

static int start_test_case(int test_case_id)
{
	void *dev;
	void *irq_load_dev = NULL;
	int hnd_idx = 0;
	BaseType_t xResult;

	os_printf("---\n\r");
	os_printf("Running test case %d:\n\r", test_case_id);

	dev = gpt_devices[0]; /* GPT1 */
	irq_load_dev = gpt_devices[1]; /* GPT2 */

	xResult = rt_latency_init(dev, irq_load_dev, &rt_ctx);
	os_assert(xResult == 0, "Initialization failed!");

	/* Initialize test case load conditions based on test case ID */
	rt_ctx.tc_load = rt_latency_get_tc_load(test_case_id);
	os_assert(rt_ctx.tc_load != -1, "Wrong test conditions!");

	/* Benchmark task: main "high prio IRQ" task */
	xResult = xTaskCreate(benchmark_task, "benchmark_task", STACK_SIZE,
			       &rt_ctx, HIGHEST_TASK_PRIORITY, &tc_taskHandles[hnd_idx++]);
	os_assert(xResult == pdPASS, "task creation failed!");

	/* CPU Load task */
	if (rt_ctx.tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		xResult = xTaskCreate(cpu_load_task, "cpu_load_task", STACK_SIZE,
				       &rt_ctx, LOWEST_TASK_PRIORITY, &tc_taskHandles[hnd_idx++]);
		os_assert(xResult == pdPASS, "task creation failed!");
	}

	/* Cache invalidate task */
	if (rt_ctx.tc_load & RT_LATENCY_WITH_INVD_CACHE) {
		xResult = xTaskCreate(cache_inval_task, "cache_inval_task",
			       STACK_SIZE, NULL, LOWEST_TASK_PRIORITY, &tc_taskHandles[hnd_idx++]);
		os_assert(xResult == pdPASS, "task creation failed!");
	}

	/* Print task */
	xResult = xTaskCreate(log_task, "log_task", STACK_SIZE,
				&rt_ctx, LOWEST_TASK_PRIORITY, &tc_taskHandles[hnd_idx++]);
	os_assert(xResult == pdPASS, "task creation failed!");

	return 0;
}

void main_task(void *pvParameters)
{
	int num = 0;
	int ret;

	os_printf("%s: running\n\r", __func__);

	do {
		if (++num >= RT_LATENCY_TEST_CASE_MAX)
			num = RT_LATENCY_TEST_CASE_1;

		ret = start_test_case(num);
		os_assert(ret == 0, "Tasks creation failed!");

		/* Execute each test case for some time */
		vTaskDelay(pdMS_TO_TICKS(TEST_EXECUTION_TIME_SEC * 1000));
		destroy_test_case();

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

	os_printf("Harpoon v.%s\r\n", VERSION);

	/* Test cases scheduler task */
	xResult = xTaskCreate(main_task, "main_task",
		       STACK_SIZE, NULL, LOWEST_TASK_PRIORITY, NULL);
	assert(xResult == pdPASS);

	/* Start scheduler */
	vTaskStartScheduler();

	for (;;)
		;


	return xResult;
}

