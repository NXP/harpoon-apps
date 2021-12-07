/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "board.h"
#include "os.h"
#include "os/assert.h"
#include "os/stdio.h"
#include "os/semaphore.h"

#include "sai_clock_config.h"
#include "sai_codec_config.h"
#include "sai_drv.h"
#include "audio.h"

#define main_task_PRIORITY	(configMAX_PRIORITIES - 3)
#define data_task_PRIORITY   (configMAX_PRIORITIES - 2)

struct mode_handler {
	void *(*init)(void *);
	void (*exit)(void *);
	int (*run)(void *);
};

struct data_ctx {
	os_sem_t semaphore;

	const struct mode_handler *handler;
	void *handle;
};

const static struct mode_handler handler[] =
{
	[0] = {
		.init = play_dtmf_init,
		.exit = play_dtmf_exit,
		.run = play_dtmf_run,
	},
	[1] = {
		.init = play_music_init,
		.exit = play_music_exit,
		.run = play_music_run,
	},
	[2] = {
		.init = play_sine_init,
		.exit = play_sine_exit,
		.run = play_sine_run,
	},
	[3] = {
		.init = rec_play_init,
		.exit = rec_play_exit,
		.run = rec_play_run,
	},
	[4] = {
		.init = rec_play2_init,
		.exit = rec_play2_exit,
		.run = rec_play2_run,
	}
};

static void hardware_setup(void)
{
	uint8_t sai_id = get_sai_id(DEMO_SAI);

	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	board_clock_setup(sai_id);
}

void data_task(void *pvParameters)
{
	struct data_ctx *ctx = pvParameters;
	bool delay;

	do {
		delay = false;

		os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);

		if (ctx->handler) {
			if (ctx->handler->run(ctx->handle) < 0)
				delay = true;
		} else {
			delay = true;
		}

		os_sem_give(&ctx->semaphore, 0);

		if (delay)
			vTaskDelay(pdMS_TO_TICKS(100));

	} while (1);
}

void main_task(void *pvParameters)
{
	struct data_ctx ctx;
	BaseType_t xResult;
	int err;

	err = os_sem_init(&ctx.semaphore, 1);
	os_assert(!err, "semaphore initialization failed!");

	ctx.handler = NULL;

	xResult = xTaskCreate(data_task, "data_task",
                        configMINIMAL_STACK_SIZE + 100, &ctx,
                        data_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "data task creation failed");

	ctx.handle = handler[DEMO_MODE].init(NULL);

	os_sem_take(&ctx.semaphore, 0, OS_SEM_TIMEOUT_MAX);
	ctx.handler = &handler[DEMO_MODE];
	os_sem_give(&ctx.semaphore, 0);

	do {
		vTaskDelay(pdMS_TO_TICKS(1000));

	} while(1);
}

int main(void)
{
	BaseType_t xResult;

	hardware_setup();

	os_printf("Audio application started!\r\n");

	xResult = xTaskCreate(main_task, "main_task",
			configMINIMAL_STACK_SIZE + 100, NULL,
			main_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "main task creation failed");

	vTaskStartScheduler();

	for (;;)
		;

	return xResult;
}
