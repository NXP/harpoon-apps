/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "app_board.h"
#include "hlog.h"
#include "os/assert.h"
#include "os/stdlib.h"
#include "sai_drv.h"
#include "codec_config.h"
#include "audio.h"

/* Use two buffers for two periods */
#define BUFFER_NUMBER		(2U)

/* Default period is when all samples words reach half of FIFO */
#define SAI_DEFAULT_PERIOD	(FSL_FEATURE_SAI_FIFO_COUNT / DEMO_AUDIO_DATA_CHANNEL / 2)

static const int supported_period[] = {2, 4, 8, 16, 32};
static const uint32_t supported_rate[] = {44100, 48000, 88200, 96000, 176400, 192000};

struct sai_statistics {
	uint64_t rec_play_periods;
	uint64_t rx_fifo_errs;
	uint64_t tx_fifo_errs;
};

struct rec_play_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;

	struct sai_device dev;
	struct sai_statistics stats;
	uint8_t *sai_buf;
	size_t period_bytes;
	uint32_t buf_index;
	sai_word_width_t bit_width;
	sai_sample_rate_t sample_rate;
	uint32_t chan_numbers;
	uint8_t period;
};

void rec_play_stats(void *handle)
{
	struct rec_play_ctx *ctx = handle;

	log_info("periods: %llu, rx error: %llu, tx error: %llu",
		ctx->stats.rec_play_periods, ctx->stats.rx_fifo_errs, ctx->stats.tx_fifo_errs);
}

static void rx_tx_callback(uint8_t status, void *user_data)
{
	struct rec_play_ctx *ctx = (struct rec_play_ctx*)user_data;

	os_assert(ctx, "userData is NULL in callback.");

	sai_disable_irq(&ctx->dev, true, false);

	ctx->event_send(ctx->event_data, status);
}

static void start_rec_play(struct rec_play_ctx *ctx)
{
	/* Disable tx/rx */
	sai_disable_tx(&ctx->dev);
	sai_disable_rx(&ctx->dev);

	/* Fill Tx FIFO with dummy data */
	memset(ctx->sai_buf, 0, ctx->period_bytes * BUFFER_NUMBER);

	/* Write two period into FIFO */
	sai_fifo_write(&ctx->dev, ctx->sai_buf,
			ctx->period_bytes * 2);

	sai_enable_tx(&ctx->dev, false);
	sai_enable_rx(&ctx->dev, true);
}

int rec_play_run(void *parameters, struct event *e)
{
	struct rec_play_ctx *ctx = (struct rec_play_ctx*)parameters;
	struct sai_device *dev = &ctx->dev;
	uint8_t *sai_buffer = ctx->sai_buf;
	size_t period_bytes = ctx->period_bytes;

	switch (e->type) {
	case EVENT_TYPE_START:
		/* Begin to record and playback */
		start_rec_play(ctx);
		break;

	case EVENT_TYPE_TX_RX:
		if (__sai_rx_error(dev->sai_base)) {
			ctx->stats.rx_fifo_errs++;
			start_rec_play(ctx);
		} else if (__sai_tx_error(dev->sai_base)) {
			ctx->stats.tx_fifo_errs++;
			start_rec_play(ctx);
		} else {
			sai_fifo_read(dev, sai_buffer +
				ctx->buf_index * period_bytes,
				period_bytes);

			sai_fifo_write(dev, sai_buffer +
				ctx->buf_index * period_bytes,
				period_bytes);

			sai_enable_irq(dev, true, false);

			ctx->buf_index++;
			if (ctx->buf_index == BUFFER_NUMBER)
				ctx->buf_index = 0;

			ctx->stats.rec_play_periods++;
		}

		break;

	default:
		break;
	}

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
	sai_config.working_mode = SAI_RX_IRQ_MODE;
	/* Set FIFO water mark to be period size of all channels*/
	sai_config.fifo_water_mark = ctx->period * ctx->chan_numbers;
	sai_config.masterSlave = DEMO_SAI_MASTER_SLAVE;

	sai_drv_setup(&ctx->dev, &sai_config);
}

void *rec_play_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	size_t frame_bytes_per_chan;
	size_t period = SAI_DEFAULT_PERIOD;
	uint32_t rate = DEMO_AUDIO_SAMPLE_RATE;
	size_t period_bytes;
	struct rec_play_ctx *ctx;
	enum codec_id cid;

	if (assign_nonzero_valid_val(period, cfg->period, supported_period) != 0) {
		log_err("Period %d frames is not supported\n", cfg->period);
		goto err;
	}

	if (assign_nonzero_valid_val(rate, cfg->rate, supported_rate) != 0) {
		log_err("Rate %d Hz is not supported\n", cfg->rate);
		goto err;
	}

	frame_bytes_per_chan = DEMO_AUDIO_BIT_WIDTH / 8;
	period_bytes = period * frame_bytes_per_chan * DEMO_AUDIO_DATA_CHANNEL;

	/* Allocate each buffer for one period */
	ctx = os_malloc(sizeof(struct rec_play_ctx) + period_bytes * BUFFER_NUMBER);
	os_assert(ctx, "Record and playback failed with memory allocation error");
	memset(ctx, 0, sizeof(struct rec_play_ctx));
	ctx->sai_buf = (uint8_t *)(ctx + 1);

	ctx->sample_rate = rate;
	ctx->chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	ctx->bit_width = DEMO_AUDIO_BIT_WIDTH;
	ctx->period = period;
	ctx->period_bytes = period_bytes;
	ctx->buf_index = 0;

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	sai_setup(ctx);

	cid = DEMO_CODEC_ID;
	codec_setup(cid);
	codec_set_format(cid, DEMO_AUDIO_MASTER_CLOCK, ctx->sample_rate,
			ctx->bit_width);

	log_info("Record and playback started (Sample Rate: %d Hz, Bit Width: %d bits, Period: %d frames)\n",
			ctx->sample_rate, ctx->bit_width, ctx->period);

	return ctx;

err:
	return NULL;
}

void rec_play_exit(void *parameters)
{
	struct rec_play_ctx *ctx = (struct rec_play_ctx*)parameters;
	enum codec_id cid;

	sai_drv_exit(&ctx->dev);

	cid = DEMO_CODEC_ID;
	codec_close(cid);

	os_free(ctx);

	log_info("\nEnd.\n");
}
