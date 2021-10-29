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

#include "rt_latency.h"
#include "rt_tc_setup.h"

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

static struct latency_stat rt_stats;

TaskHandle_t main_taskHandle;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Application tasks */
void main_task(void *pvParameters);
void log_task(void *pvParameters);
#ifdef WITH_CPU_LOAD
void cpu_load_task(void *pvParameters);
#endif
#ifdef WITH_INVD_CACHE
void cache_inval_task(void *pvParameters);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#ifdef WITH_CPU_LOAD
void cpu_load_task(void *pvParameters)
{
	cpu_load();
}
#endif

#ifdef WITH_INVD_CACHE
void cache_inval_task(void *pvParameters)
{
	cache_inval();
}
#endif

void log_task(void *pvParameters)
{
	struct latency_stat *pStats = pvParameters;

	print_stats(pStats);
}

void main_task(void *pvParameters)
{
	int ret;
	struct latency_stat *rt_stat = pvParameters;
	char *irq_load_msg =
#ifdef WITH_IRQ_LOAD
		" (with IRQ load)";
#else
		"";
#endif
	os_printf("%s: task started%s\n\r", __func__, irq_load_msg);

	ret = rt_latency_test(rt_stat);
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
	BaseType_t xResult;

	/* Init board cpu and hardware. */
	BOARD_InitMemory();
	BOARD_InitDebugConsole();

	/* Create (main) "high prio IRQ" task */

	dev = gpt_devices[0]; /* GPT1 */

#ifdef WITH_IRQ_LOAD
	irq_load_dev = gpt_devices[1]; /* GPT2 */
#endif

	xResult = rt_latency_init(dev, irq_load_dev, &rt_stats);
	os_assert(xResult == 0, "Initialization failed!");

	xResult = xTaskCreate(main_task, "main_task", STACK_SIZE,
			       &rt_stats, HIGHEST_TASK_PRIORITY, &main_taskHandle);
	if (xResult != pdPASS);
		assert (true);

	/* CPU Load task */
#ifdef WITH_CPU_LOAD
	xResult = xTaskCreate(cpu_load_task, "cpu_load_task", STACK_SIZE,
			       NULL, LOWEST_TASK_PRIORITY, NULL);
	assert(xResult == pdPASS);
#endif

	/* Cache invalidate task */
#ifdef WITH_INVD_CACHE
	xResult = xTaskCreate(cache_inval_task, "cache_inval_task", STACK_SIZE,
			       NULL, LOWEST_TASK_PRIORITY, NULL);
	assert(xResult == pdPASS);
#endif

	/* Print task */
	xResult = xTaskCreate(log_task, "log_task", STACK_SIZE,
				&rt_stats, LOWEST_TASK_PRIORITY, NULL);
	assert(xResult == pdPASS);

	/* Start scheduler */
	os_printf("starting scheduler...\n\r");
	vTaskStartScheduler();

	for (;;)
		;
}
