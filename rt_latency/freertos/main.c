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
#include "os/counter.h"
#include "os/semaphore.h"
#include "os/stdio.h"

#include "stats.h"
#include "ivshmem.h"
#include "hrpn_ctrl.h"
#include "mailbox.h"
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

	os_printf("%s: running%s\r\n", __func__,
		ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD_SEM ? " (with extra semaphore load)" : "");

	do {
		cpu_load(ctx);
	} while(1);
}

void cache_inval_task(void *pvParameters)
{
	os_printf("%s: running\r\n", __func__);

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

	os_printf("%s: running%s\r\n", __func__,
	       (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD)  ? " (with IRQ load)" : "");

	do {
		ret = rt_latency_test(ctx);
		if (ret)
		{
			os_printf("test failed!\n");
			vTaskSuspend(NULL);
		}
	} while (!ret);
}

/*******************************************************************************
 * Application functions
 ******************************************************************************/

static void destroy_test_case(struct main_ctx *ctx)
{
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

static int start_test_case(struct main_ctx *ctx, int test_case_id)
{
	void *dev;
	void *irq_load_dev = NULL;
	int hnd_idx = 0;
	BaseType_t xResult;

	if (ctx->started)
		return -1;

	os_printf("---\r\n");
	os_printf("Running test case %d:\r\n", test_case_id);

	dev = gpt_devices[0]; /* GPT1 */
	irq_load_dev = gpt_devices[1]; /* GPT2 */

	/* Initialize test case load conditions based on test case ID */
	ctx->rt_ctx.tc_load = rt_latency_get_tc_load(test_case_id);
	if (ctx->rt_ctx.tc_load < 0) {
		os_printf("Wrong test conditions!\r\n");
		goto err;
	}

	/* Initialize test cases' context */
	xResult = rt_latency_init(dev, irq_load_dev, &ctx->rt_ctx);
	if (xResult) {
		os_printf("Initialization failed!\r\n");
		goto err;
	}

	/* Benchmark task: main "high prio IRQ" task */
	xResult = xTaskCreate(benchmark_task, "benchmark_task", STACK_SIZE,
			       &ctx->rt_ctx, HIGHEST_TASK_PRIORITY - 1, &ctx->tc_taskHandles[hnd_idx++]);
	if (xResult != pdPASS) {
		os_printf("task creation failed!\r\n");
		goto err_task;
	}

	/* CPU Load task */
	if (ctx->rt_ctx.tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		xResult = xTaskCreate(cpu_load_task, "cpu_load_task", STACK_SIZE,
				       &ctx->rt_ctx, LOWEST_TASK_PRIORITY, &ctx->tc_taskHandles[hnd_idx++]);
		if (xResult != pdPASS) {
			os_printf("task creation failed!\r\n");
			goto err_task;
		}
	}

	/* Cache invalidate task */
	if (ctx->rt_ctx.tc_load & RT_LATENCY_WITH_INVD_CACHE) {
		xResult = xTaskCreate(cache_inval_task, "cache_inval_task",
			       STACK_SIZE, NULL, LOWEST_TASK_PRIORITY + 1, &ctx->tc_taskHandles[hnd_idx++]);
		if (xResult != pdPASS) {
			os_printf("task creation failed!\r\n");
			goto err_task;
		}
	}

	/* Print task */
	xResult = xTaskCreate(log_task, "log_task", STACK_SIZE,
				&ctx->rt_ctx, LOWEST_TASK_PRIORITY + 1, &ctx->tc_taskHandles[hnd_idx++]);
	if (xResult != pdPASS) {
		os_printf("task creation failed!\r\n");
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

static void response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_latency resp;

	resp.type = HRPN_RESP_TYPE_LATENCY;
	resp.status = status;
	mailbox_resp_send(m, &resp, sizeof(resp));
}

static void command_handler(struct main_ctx *ctx, struct mailbox *m)
{
	struct hrpn_command cmd;
	unsigned int len;
	int ret;

	len = sizeof(cmd);
	if (mailbox_cmd_recv(m, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_LATENCY_RUN:
		if (len != sizeof(struct hrpn_cmd_latency_run)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		if (cmd.u.latency_run.id >= RT_LATENCY_TEST_CASE_MAX) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		ret = start_test_case(ctx, cmd.u.latency_run.id);
		if (ret)
			response(m, HRPN_RESP_STATUS_ERROR);
		else
			response(m, HRPN_RESP_STATUS_SUCCESS);

		break;

	case HRPN_CMD_TYPE_LATENCY_STOP:
		if (len != sizeof(struct hrpn_cmd_latency_stop)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		destroy_test_case(ctx);
		response(m, HRPN_RESP_STATUS_SUCCESS);
		break;

	default:
		response(m, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

void main_task(void *pvParameters)
{
	struct main_ctx *ctx = pvParameters;
	struct ivshmem mem;
	struct mailbox m;
	int rc;

	os_printf("%s: running\r\n", __func__);

	rc = ivshmem_init(0, &mem);
	os_assert(!rc, "ivshmem initialization failed, can not proceed\r\n");

	os_assert(mem.out_size, "ivshmem mis-configuration, can not proceed\r\n");

	mailbox_init(&m, mem.out, mem.out + mem.out_size * mem.id, false);

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

	os_printf("Harpoon v.%s\r\n", VERSION);

	/* Test cases scheduler task */
	xResult = xTaskCreate(main_task, "main_task",
		       STACK_SIZE, &main_ctx, LOWEST_TASK_PRIORITY + 1, NULL);
	assert(xResult == pdPASS);

	/* Start scheduler */
	vTaskStartScheduler();

	for (;;)
		;


	return xResult;
}

