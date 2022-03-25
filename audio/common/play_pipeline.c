/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/stdlib.h"

#include "board.h"

#include "audio_pipeline.h"
#include "audio.h"
#include "log.h"
#include "sai_codec_config.h"
#include "sai_drv.h"

extern struct audio_pipeline_config pipeline_config;

#define DEFAULT_PERIOD		8
#define DEFAULT_SAMPLE_RATE	48000

static const int supported_period[] = {2, 4, 8, 16, 32};
static const uint32_t supported_rate[] = {44100, 48000, 88200, 96000, 176400, 192000};

struct pipeline_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;

	struct sai_device dev;
	struct audio_pipeline *pipeline;
	sai_word_width_t bit_width;
	sai_sample_rate_t sample_rate;
	uint32_t chan_numbers;
	uint8_t period;

	struct {
		uint64_t callback;
		uint64_t run;
		uint64_t err;
	} stats;
};

void play_pipeline_stats(void *handle)
{
	struct pipeline_ctx *ctx = handle;

	log_info("callback: %llu, run: %llu, err: %llu\n", ctx->stats.callback, ctx->stats.run,
		ctx->stats.err);
}

static void rx_callback(uint8_t status, void *user_data)
{
	struct pipeline_ctx *ctx = (struct pipeline_ctx*)user_data;

	sai_disable_irq(&ctx->dev, true, false);

	ctx->stats.callback++;

	ctx->event_send(ctx->event_data, status);
}

int play_pipeline_run(void *handle, struct event *e)
{
	struct pipeline_ctx *ctx = handle;
	int err;

	ctx->stats.run++;

	err = audio_pipeline_run(ctx->pipeline);
	if (err) {
		ctx->stats.err++;
		audio_pipeline_reset(ctx->pipeline);
		err = audio_pipeline_run(ctx->pipeline);
		os_assert(!err, "pipeline couldn't restart");
	}

	sai_enable_irq(&ctx->dev, true, false);

	return err;
}

static void sai_setup(struct pipeline_ctx *ctx)
{
	struct sai_cfg sai_config;

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = ctx->bit_width;
	sai_config.sample_rate = ctx->sample_rate;
	sai_config.chan_numbers = ctx->chan_numbers;
	sai_config.source_clock_hz = DEMO_AUDIO_MASTER_CLOCK;
	sai_config.tx_sync_mode = DEMO_SAI_TX_SYNC_MODE;
	sai_config.rx_sync_mode = DEMO_SAI_RX_SYNC_MODE;
	sai_config.rx_callback = rx_callback;
	sai_config.rx_user_data = ctx;
	sai_config.working_mode = SAI_CONTINUE_MODE;
	sai_config.masterSlave = DEMO_SAI_MASTER_SLAVE;
	/* Set FIFO water mark to be period size of all channels*/
	sai_config.fifo_water_mark = ctx->period * ctx->chan_numbers;

	sai_drv_setup(&ctx->dev, &sai_config);
}

void *play_pipeline_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	struct pipeline_ctx *ctx;
	size_t period = DEFAULT_PERIOD;
	uint32_t rate = DEFAULT_SAMPLE_RATE;
	enum codec_id cid;

	if (assign_nonzero_valid_val(period, cfg->period, supported_period) != 0) {
		log_err("Period %d frames is not supported\n", cfg->period);
		goto err;
	}

	if (assign_nonzero_valid_val(rate, cfg->rate, supported_rate) != 0) {
		log_err("Rate %d Hz is not supported\n", cfg->rate);
		goto err;
	}

	ctx = os_malloc(sizeof(struct pipeline_ctx));
	os_assert(ctx, "Audio pipeline failed with memory allocation error");
	memset(ctx, 0, sizeof(struct pipeline_ctx));

	/* override pipeline configuration */
	pipeline_config.sample_rate = rate;
	pipeline_config.period = period;

	ctx->pipeline = audio_pipeline_init(&pipeline_config);
	if (!ctx->pipeline)
		goto err_init;

	audio_pipeline_dump(ctx->pipeline);

	ctx->sample_rate = rate;
	ctx->chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	ctx->bit_width = DEMO_AUDIO_BIT_WIDTH;
	ctx->period = period;
	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	sai_setup(ctx);

#ifdef WM8960_SAI_CLK_FREQ
	cid = CODEC_ID_WM8960;
	codec_setup(cid);
	codec_set_format(cid, WM8960_SAI_CLK_FREQ, rate,
			ctx->bit_width);
#endif

#ifdef WM8524_SAI_CLK_FREQ
	cid = CODEC_ID_WM8524;
	codec_setup(cid);
	codec_set_format(cid, WM8524_SAI_CLK_FREQ, rate,
			ctx->bit_width);
#endif

	cid = CODEC_ID_HIFIBERRY;
	codec_setup(cid);
	codec_set_format(cid, HIFIBERRY_SAI_CLK_FREQ, rate,
			ctx->bit_width);

	log_info("Starting pipeline (Sample Rate: %d Hz, Period: %d frames)\n",
			rate, period);

	return ctx;

err_init:
	os_free(ctx);

err:
	return NULL;
}

void play_pipeline_exit(void *handle)
{
	struct pipeline_ctx *ctx = handle;
	enum codec_id cid;

	audio_pipeline_exit(ctx->pipeline);

	sai_drv_exit(&ctx->dev);

#ifdef WM8960_SAI_CLK_FREQ
	cid = CODEC_ID_WM8960;
	codec_close(cid);
#endif

#ifdef WM8524_SAI_CLK_FREQ
	cid = CODEC_ID_WM8524;
	codec_close(cid);
#endif

	cid = CODEC_ID_HIFIBERRY;
	codec_close(cid);

	os_free(ctx);

	log_info("\nEnd.\n");
}
