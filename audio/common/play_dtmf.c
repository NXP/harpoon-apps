/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "board.h"
#include "os/assert.h"
#include "os/stdlib.h"
#include "sai_drv.h"
#include "dtmf_wave.h"
#include "sai_codec_config.h"
#include "audio.h"

#define DTMF_AUDIO_SRATE 44100 /* default sampling rate */
#define DTMF_AUDIO_BITWIDTH 32
#define DTMF_TONE_DURATION_MS 120

static const uint32_t supported_rate[] = {44100, 48000, 88200, 96000, 176400, 192000};

static const char default_dtmf_l_seq[] = "1123ABCD0123456789*#";
static const char default_dtmf_r_seq[] = "#*9876543210DCBA3211";

struct dtmf_ctx {
	uint32_t *audio_buf;

	void (*event_send)(void *, uint8_t);
	void *event_data;

	struct sai_device dev;

	const char *dtmf_l_seq;
	const char *dtmf_r_seq;

	bool playing;

	uint32_t sample_rate;
	size_t audio_buf_size;
	unsigned int dtmf_seq_idx;
	uint32_t phase;

	uint32_t play_times;
};

void play_dtmf_stats(void *handle)
{
	struct dtmf_ctx *ctx = handle;

	os_printf("Played DTMF sequence: %d times\r", ctx->play_times);
}

static void tx_callback(uint8_t status, void *userData)
{
	struct dtmf_ctx *ctx = userData;

	ctx->event_send(ctx->event_data, status);
}

int play_dtmf_run(void *handle, struct event *e)
{
	struct dtmf_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	int err = 0;

	if (ctx->playing) {
		/* prepare blank buffer */
		memset(ctx->audio_buf, 0, ctx->audio_buf_size);

		/* transmit audio buffer */
		err = sai_write(dev, (uint8_t *)ctx->audio_buf, ctx->audio_buf_size);

		ctx->playing = false;
	} else {
		/* prepare dtmf audio buffer */
		generate_dtmf_tone(ctx->audio_buf, ctx->audio_buf_size,
				ctx->dtmf_l_seq[ctx->dtmf_seq_idx],
				ctx->dtmf_r_seq[ctx->dtmf_seq_idx],
				ctx->sample_rate, DTMF_TONE_DURATION_MS,
				&ctx->phase);

		ctx->dtmf_seq_idx++;
		if (ctx->dtmf_seq_idx >= strlen(ctx->dtmf_l_seq))
			ctx->dtmf_seq_idx = 0;

		ctx->play_times++;

		/* transmit audio buffer */
		err = sai_write(dev, (uint8_t *)ctx->audio_buf, ctx->audio_buf_size);

		ctx->playing = true;
	}

	return err;
}

static void sai_setup(struct dtmf_ctx *ctx, struct audio_config *cfg)
{
	struct sai_cfg sai_config = {0};

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = DTMF_AUDIO_BITWIDTH;
	sai_config.sample_rate = ctx->sample_rate;
	sai_config.chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	sai_config.source_clock_hz = DEMO_AUDIO_MASTER_CLOCK;
	sai_config.tx_sync_mode = DEMO_SAI_TX_SYNC_MODE;
	sai_config.rx_sync_mode = DEMO_SAI_RX_SYNC_MODE;
	sai_config.tx_callback = tx_callback;
	sai_config.tx_user_data = ctx;

	sai_drv_setup(&ctx->dev, &sai_config);
}

void *play_dtmf_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	struct dtmf_ctx *ctx;
	size_t buffer_bytes;
	uint32_t rate = DTMF_AUDIO_SRATE;

	if (assign_nonzero_valid_val(rate, cfg->rate, supported_rate) != 0) {
		os_printf("Frequency %d Hz is not supported\r\n", cfg->rate);
		goto err;
	}

	buffer_bytes = rate * 2 * (DTMF_AUDIO_BITWIDTH / 8) *
			DTMF_TONE_DURATION_MS / 1000;

	ctx = os_malloc(sizeof(struct dtmf_ctx) + buffer_bytes);
	os_assert(ctx, "Playing DTMF failed with memory allocation error");
	memset(ctx, 0, sizeof(struct dtmf_ctx) + buffer_bytes);
	ctx->audio_buf = (uint32_t *)(ctx + 1);
	ctx->audio_buf_size = buffer_bytes;

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	ctx->dtmf_l_seq = default_dtmf_l_seq;
	ctx->dtmf_r_seq = default_dtmf_r_seq;

	ctx->sample_rate = rate;

	ctx->dtmf_seq_idx = 0;
	ctx->phase = 0;
	ctx->playing = false;

	sai_setup(ctx, cfg);

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, ctx->sample_rate, DTMF_AUDIO_BITWIDTH);

	os_printf("Playing DTMF sequence (Sample Rate: %d Hz, Bit Width: %d bits)\r\n", ctx->sample_rate, DTMF_AUDIO_BITWIDTH);
	os_printf("\tleft channel:  %s\r\n", ctx->dtmf_l_seq);
	os_printf("\tright channel: %s\r\n", ctx->dtmf_r_seq);

	return ctx;

err:
	return NULL;
}

void play_dtmf_exit(void *handle)
{
	struct dtmf_ctx *ctx = handle;

	sai_drv_exit(&ctx->dev);

	codec_close();

	os_free(ctx);

	os_printf("\r\nEnd.\r\n");
}
