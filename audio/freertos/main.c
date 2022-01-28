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

#include "ivshmem.h"
#include "mailbox.h"
#include "hrpn_ctrl.h"

#include "pin_mux.h"
#include "sai_clock_config.h"
#include "sai_codec_config.h"
#include "sai_drv.h"
#include "audio.h"

#define main_task_PRIORITY	(configMAX_PRIORITIES - 3)
#define data_task_PRIORITY   (configMAX_PRIORITIES - 2)

struct mode_handler {
	void *(*init)(void *);
	void (*exit)(void *);
	void (*stats)(void *);
	int (*run)(void *, struct event *e);
};

struct data_ctx {
	os_sem_t semaphore;

	QueueHandle_t event_queue_h;

	const struct mode_handler *handler;
	void *handle;
};

const static struct mode_handler handler[] =
{
	[0] = {
		.init = play_dtmf_init,
		.exit = play_dtmf_exit,
		.run = play_dtmf_run,
		.stats = play_dtmf_stats,
	},
	[1] = {
		.init = play_music_init,
		.exit = play_music_exit,
		.run = play_music_run,
		.stats = play_music_stats,
	},
	[2] = {
		.init = play_sine_init,
		.exit = play_sine_exit,
		.run = play_sine_run,
		.stats = play_sine_stats,
	},
	[3] = {
		.init = rec_play_init,
		.exit = rec_play_exit,
		.run = rec_play_run,
		.stats = rec_play_stats,
	},
	[4] = {
		.init = rec_play2_init,
		.exit = rec_play2_exit,
		.run = rec_play2_run,
		.stats = rec_play2_stats,
	}
};

static void hardware_setup(void)
{
	uint8_t sai_id = get_sai_id(DEMO_SAI);

	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	BOARD_InitPins();

	board_clock_setup(sai_id);
}

static void data_send_event(void *userData, uint8_t status)
{
	QueueHandle_t event_queue_h = userData;
	BaseType_t wake = pdFALSE;
	struct event e;

	e.type = EVENT_TYPE_TX_RX;
	e.data = status;

	xQueueSendToBackFromISR(event_queue_h, &e, &wake);
	portYIELD_FROM_ISR(wake);
}

void data_task(void *pvParameters)
{
	struct data_ctx *ctx = pvParameters;
	struct event e;

	do {
		/* check event */
		if (xQueueReceive(ctx->event_queue_h, &e, portMAX_DELAY) != pdTRUE)
			continue;

		os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);

		if (ctx->handler)
			ctx->handler->run(ctx->handle, &e);

		os_sem_give(&ctx->semaphore, 0);

	} while (1);
}

static void response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio resp;

	resp.type = HRPN_RESP_TYPE_AUDIO;
	resp.status = status;
	mailbox_resp_send(m, &resp, sizeof(resp));
}

static int audio_run(struct data_ctx *ctx, struct hrpn_cmd_audio_run *run)
{
	int rc = HRPN_RESP_STATUS_ERROR;
	struct audio_config cfg;
	struct event e;

	if (ctx->handler)
		goto exit;

	if (run->id >= ARRAY_SIZE(handler))
		goto exit;

	cfg.event_send = data_send_event;
	cfg.event_data = ctx->event_queue_h;
	cfg.rate = run->frequency;
	cfg.period = run->period;

	ctx->handle = handler[run->id].init(&cfg);
	if (!ctx->handle)
		goto exit;

	os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	ctx->handler = &handler[run->id];
	os_sem_give(&ctx->semaphore, 0);

	/* Send an event to trigger data thread processing */
	e.type = EVENT_TYPE_START;

	xQueueSendToBack(ctx->event_queue_h, &e, 0);

	rc = HRPN_RESP_STATUS_SUCCESS;

exit:
	return rc;
}

static int audio_stop(struct data_ctx *ctx)
{
	const struct mode_handler *handler;

	if (!ctx->handler)
		goto exit;

	os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	handler = ctx->handler;
	ctx->handler = NULL;
	os_sem_give(&ctx->semaphore, 0);

	handler->exit(ctx->handle);

exit:
	return HRPN_RESP_STATUS_SUCCESS;
}

static void audio_stats(struct data_ctx *ctx)
{
	if (ctx->handler)
		ctx->handler->stats(ctx->handle);
}

static void command_handler(struct mailbox *m, struct data_ctx *ctx)
{
	struct hrpn_command cmd;
	unsigned int len;
	int rc;

	len = sizeof(cmd);
	if (mailbox_cmd_recv(m, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_AUDIO_RUN:
		if (len != sizeof(struct hrpn_cmd_audio_run)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = audio_run(ctx, &cmd.u.audio_run);

		response(m, rc);

		break;

	case HRPN_CMD_TYPE_AUDIO_STOP:
		if (len != sizeof(struct hrpn_cmd_audio_stop)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = audio_stop(ctx);

		response(m, rc);

		break;

	default:
		response(m, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

#define CONTROL_POLL_PERIOD	100
#define STATS_POLL_PERIOD	10000
#define STATS_COUNT		(STATS_POLL_PERIOD / CONTROL_POLL_PERIOD)

void main_task(void *pvParameters)
{
	struct data_ctx ctx;
	struct ivshmem mem;
	struct mailbox m;
	BaseType_t xResult;
	int count;
	int err;
	int rc;

	rc = ivshmem_init(0, &mem);
	os_assert(!rc, "ivshmem initialization failed, can not proceed\r\n");

	os_assert(mem.out_size, "ivshmem mis-configuration, can not proceed\r\n");

	mailbox_init(&m, mem.out, mem.out + mem.out_size * mem.id, false);

	err = os_sem_init(&ctx.semaphore, 1);
	os_assert(!err, "semaphore initialization failed!");

	ctx.event_queue_h = xQueueCreate(10, sizeof(struct event));
	os_assert(ctx.event_queue_h, "event queue creation failed");

	ctx.handler = NULL;

	xResult = xTaskCreate(data_task, "data_task",
                        configMINIMAL_STACK_SIZE + 100, &ctx,
                        data_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "data task creation failed");

	count = STATS_COUNT;
	do {
		command_handler(&m, &ctx);

		count--;
		if (!count) {
			audio_stats(&ctx);
			count = STATS_COUNT;
		}

		vTaskDelay(pdMS_TO_TICKS(CONTROL_POLL_PERIOD));

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
