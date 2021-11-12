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

TaskHandle_t main_taskHandle;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Application tasks */
void main_task(void *pvParameters);
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

void main_task(void *pvParameters)
{
	int ret;
	struct rt_latency_ctx *ctx = pvParameters;

	os_printf("%s: task started%s\n\r", __func__,
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

/*!
 * @brief Main function
 */
int main(void)
{
	void *dev;
	void *irq_load_dev = NULL;
	int test_case_id = RT_LATENCY_TEST_CASE_7;
	BaseType_t xResult;

	/* Init board cpu and hardware. */
	BOARD_InitMemory();
	BOARD_InitDebugConsole();

	os_printf("Harpoon v.%s\r\n", VERSION);

	/* Create (main) "high prio IRQ" task */

	dev = gpt_devices[0]; /* GPT1 */
	irq_load_dev = gpt_devices[1]; /* GPT2 */

	/* Initialize test case load conditions based on test case ID */
	rt_ctx.tc_load = rt_latency_get_tc_load(test_case_id);
	os_assert(rt_ctx.tc_load != -1, "Wrong test conditions!");

	xResult = rt_latency_init(dev, irq_load_dev, &rt_ctx);
	os_assert(xResult == 0, "Initialization failed!");

	xResult = xTaskCreate(main_task, "main_task", STACK_SIZE,
			       &rt_ctx, HIGHEST_TASK_PRIORITY, &main_taskHandle);
	if (xResult != pdPASS);
		assert (true);

	/* CPU Load task */
	if (rt_ctx.tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		xResult = xTaskCreate(cpu_load_task, "cpu_load_task", STACK_SIZE,
				       &rt_ctx, LOWEST_TASK_PRIORITY, NULL);
		assert(xResult == pdPASS);
	}

	/* Cache invalidate task */
	if (rt_ctx.tc_load & RT_LATENCY_WITH_INVD_CACHE) {
		xResult = xTaskCreate(cache_inval_task, "cache_inval_task",
			       STACK_SIZE, NULL, LOWEST_TASK_PRIORITY, NULL);
		assert(xResult == pdPASS);
	}

	/* Print task */
	xResult = xTaskCreate(log_task, "log_task", STACK_SIZE,
				&rt_ctx, LOWEST_TASK_PRIORITY, NULL);
	assert(xResult == pdPASS);

	/* Start scheduler */
	os_printf("Running test case %d\n\r", test_case_id);
	vTaskStartScheduler();

	for (;;)
		;

	return xResult;
}

