/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "board.h"
#include "os/assert.h"
#include "os/stdlib.h"
#include "sai_drv.h"
#include "sai_codec_config.h"

#define BUFFER_NUMBER		(2U)

#define SAI_EVENT_QUEUE_LENGTH	16

enum event_type {
	SAI_EVENT_IRQ = 0,
};

struct event {
	unsigned int type;
	uint8_t data;
};

struct sai_statistics {
	uint64_t rec_play_periods;
	uint64_t rx_fifo_errs;
	uint64_t tx_fifo_errs;
	uint64_t queue_errs;
};

struct rec_play_ctx {
	struct sai_device dev;
	struct sai_statistics stats;
	uint8_t *sai_buf;
	size_t period_bytes_per_chan;
	size_t buffer_size;
	uint32_t run_time;	/* ms, 0 for run for ever */
	uint32_t buf_index;
	uint8_t irq_status;
	QueueHandle_t event_queue_h;
	sai_word_width_t bit_width;
	sai_sample_rate_t sample_rate;
	uint32_t chan_numbers;
};

static void rx_tx_callback(uint8_t status, void *user_data)
{
	struct event e;
	BaseType_t wake = pdFALSE;
	BaseType_t ret;
	struct rec_play_ctx *ctx = (struct rec_play_ctx*)user_data;

	os_assert(ctx, "userData is NULL in callback.");

	sai_disable_irq(&ctx->dev, true, false);

	e.type = SAI_EVENT_IRQ;
	e.data = status;
	ret = xQueueSendToBackFromISR(ctx->event_queue_h, &e, &wake);
	if (ret != pdPASS)
		ctx->stats.queue_errs++;
	if (wake)
		portYIELD_FROM_ISR(wake);
}

static void start_rec_play(struct rec_play_ctx *ctx)
{
	/* Reset FIFO for safe */
	reset_tx_fifo(&ctx->dev);
	reset_rx_fifo(&ctx->dev);
	/* Fill Tx FIFO with dumy data */
	memset(ctx->sai_buf, 0, sizeof(ctx->sai_buf));
	/* Write two period into FIFO */
	sai_fifo_write(&ctx->dev, ctx->sai_buf,
			ctx->period_bytes_per_chan * 2);
	sai_enable_tx(&ctx->dev, false);
	sai_enable_rx(&ctx->dev, true);
}

int rec_play_run(void *parameters)
{
	struct rec_play_ctx *ctx = (struct rec_play_ctx*)parameters;
	struct sai_device *dev = &ctx->dev;
	uint8_t *sai_buffer = ctx->sai_buf;
	uint32_t end_time = 0;
	size_t period_bytes_per_chan = ctx->period_bytes_per_chan;
	size_t buffer_size = ctx->buffer_size;
	struct event sai_event;
	uint8_t status;

	/* Begin to record and playback */
	start_rec_play(ctx);

	if (ctx->run_time != 0)
		end_time = xTaskGetTickCount() + ctx->run_time;

	do {
		if (xQueueReceive(ctx->event_queue_h, &sai_event, portMAX_DELAY)
				!= pdTRUE)
			continue;

		status = sai_event.data;
		switch (sai_event.type) {
		case SAI_EVENT_IRQ:
			if (status == SAI_STATUS_NO_ERROR) {
				sai_fifo_read(dev, sai_buffer +
						ctx->buf_index * buffer_size,
						period_bytes_per_chan);
				sai_fifo_write(dev, sai_buffer +
						ctx->buf_index * buffer_size,
						period_bytes_per_chan);
				sai_enable_irq(dev, true, false);
				ctx->stats.rec_play_periods++;
			} else {
				/* Restart the process in case of error */
				start_rec_play(ctx);
				if (status & SAI_STATUS_TX_FF_ERR)
					ctx->stats.tx_fifo_errs++;
				if (status & SAI_STATUS_RX_FF_ERR)
					ctx->stats.rx_fifo_errs++;
			}
			break;
		default:
			break;
		}

		ctx->buf_index++;
		if (ctx->buf_index == BUFFER_NUMBER)
			ctx->buf_index = 0;

		if (end_time != 0 && xTaskGetTickCount() > end_time)
			break;
	} while (1);

	return 0;
}

static void sai_setup(struct rec_play_ctx *ctx)
{
	struct sai_cfg sai_config;

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = ctx->bit_width;
	sai_config.sample_rate = ctx->sample_rate;
	sai_config.chan_numbers = ctx->chan_numbers;
	sai_config.source_clock_hz = DEMO_AUDIO_MASTER_CLOCK;
	sai_config.tx_sync_mode = DEMO_SAI_TX_SYNC_MODE;
	sai_config.rx_sync_mode = DEMO_SAI_RX_SYNC_MODE;
	sai_config.rx_callback = rx_tx_callback;
	sai_config.rx_user_data = ctx;
	sai_config.working_mode = SAI_CONTINUE_MODE;

	sai_drv_setup(&ctx->dev, &sai_config);
}

void *rec_play_init(void *parameters)
{
	size_t frame_bytes_per_chan;
	size_t period_size = FSL_FEATURE_SAI_FIFO_COUNT / 2;
	size_t period_bytes_per_chan;
	size_t buffer_size;
	struct rec_play_ctx *ctx;

	frame_bytes_per_chan = DEMO_AUDIO_BIT_WIDTH / 8;
	period_bytes_per_chan = period_size * frame_bytes_per_chan;
	buffer_size = period_bytes_per_chan * DEMO_AUDIO_DATA_CHANNEL;

	ctx = os_malloc(sizeof(struct rec_play_ctx) + buffer_size * BUFFER_NUMBER);
	os_assert(ctx, "Record and playback failed with memory allocation error");
	memset(ctx, 0, sizeof(struct rec_play_ctx));
	ctx->sai_buf = (uint8_t *)(ctx + 1);

	ctx->sample_rate = DEMO_AUDIO_SAMPLE_RATE;
	ctx->chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	ctx->bit_width = DEMO_AUDIO_BIT_WIDTH;
	ctx->period_bytes_per_chan = period_bytes_per_chan;
	ctx->buffer_size = buffer_size;
	ctx->run_time = 60000;	/* 60s */
	ctx->buf_index = 0;

	ctx->event_queue_h = xQueueCreate(SAI_EVENT_QUEUE_LENGTH,
				sizeof(struct event));
	os_assert(ctx->event_queue_h, "Can't create event queue.");

	sai_setup(ctx);

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, ctx->sample_rate,
			ctx->bit_width);

	os_printf("HifiBerry record playback demo is started (Sample Rate: %d Hz, Bit Width: %d bits)\r\n",
			ctx->sample_rate, ctx->bit_width);

	return ctx;
}

void rec_play_exit(void *parameters)
{
	struct rec_play_ctx *ctx = (struct rec_play_ctx*)parameters;

	sai_drv_exit(&ctx->dev);

	codec_close();

	os_free(ctx);

	os_printf("End.\r\n");
}
