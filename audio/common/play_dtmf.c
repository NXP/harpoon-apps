/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "os/assert.h"
#include "os/semaphore.h"
#include "os/stdlib.h"
#include "sai_drv.h"
#include "dtmf_wave.h"
#include "sai_codec_config.h"

#define DTMF_AUDIO_SRATE 44100 /* default sampling rate */
#define DTMF_AUDIO_BITWIDTH 32
#define DTMF_TONE_DURATION_MS 120

#define BUFFER_BYTES (DTMF_AUDIO_SRATE * 2 * (DTMF_AUDIO_BITWIDTH / 8) \
		* DTMF_TONE_DURATION_MS / 1000)

static const char default_dtmf_l_seq[] = "1123ABCD0123456789*#";
static const char default_dtmf_r_seq[] = "#*9876543210DCBA3211";

struct dtmf_ctx {
	uint32_t audio_buf[BUFFER_BYTES / 4];

	/* callback semaphore */
	os_sem_t tx_semaphore;
	os_sem_t rx_semaphore;

	struct sai_device dev;

	const char *dtmf_l_seq;
	const char *dtmf_r_seq;

	int sample_rate;
	size_t audio_buf_size;
	unsigned int dtmf_seq_idx;
	uint32_t phase;
};

static void rx_callback(uint8_t status, void *userData)
{
	struct dtmf_ctx *ctx = userData;

	if (status == SAI_STATUS_NO_ERROR)
		os_sem_give(&ctx->rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void tx_callback(uint8_t status, void *userData)
{
	struct dtmf_ctx *ctx = userData;

	if (status == SAI_STATUS_NO_ERROR)
		os_sem_give(&ctx->tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static int play_dtmf_run(void *handle)
{
	struct dtmf_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	int err;

	while (1) {
		/* prepare dtmf audio buffer */
		generate_dtmf_tone(ctx->audio_buf, ctx->dtmf_l_seq[ctx->dtmf_seq_idx],
				ctx->dtmf_r_seq[ctx->dtmf_seq_idx], ctx->sample_rate,
				DTMF_TONE_DURATION_MS, &ctx->phase);

		ctx->dtmf_seq_idx++;
		if (ctx->dtmf_seq_idx >= strlen(ctx->dtmf_l_seq))
			ctx->dtmf_seq_idx = 0;

		/* transmit audio buffer */
		err = sai_write(dev, (uint8_t *)ctx->audio_buf, ctx->audio_buf_size);
		if (!err) {
			err = os_sem_take(&ctx->tx_semaphore, 0,
					OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)",
					err);
		}

		/* prepare blank buffer */
		memset(ctx->audio_buf, 0, ctx->audio_buf_size);

		/* transmit audio buffer */
		err = sai_write(dev, (uint8_t *)ctx->audio_buf, ctx->audio_buf_size);
		if (!err) {
			err = os_sem_take(&ctx->tx_semaphore, 0,
					OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)",
					err);
		}
	}

	return 0;
}

static void sai_setup(struct dtmf_ctx *ctx)
{
	struct sai_cfg sai_config;

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = DTMF_AUDIO_BITWIDTH;
	sai_config.sample_rate = DTMF_AUDIO_SRATE;
	sai_config.chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	sai_config.source_clock_hz = DEMO_AUDIO_MASTER_CLOCK;
	sai_config.tx_sync_mode = DEMO_SAI_TX_SYNC_MODE;
	sai_config.rx_sync_mode = DEMO_SAI_RX_SYNC_MODE;
	sai_config.rx_callback = rx_callback;
	sai_config.rx_user_data = ctx;
	sai_config.tx_callback = tx_callback;
	sai_config.tx_user_data = ctx;

	sai_drv_setup(&ctx->dev, &sai_config);
}

static void *play_dtmf_init(void *parameters)
{
	struct dtmf_ctx *ctx;
	int err;

	ctx = os_malloc(sizeof(struct dtmf_ctx));
	os_assert(ctx, "Playing DTMF failed with memory allocation error");

	ctx->audio_buf_size = BUFFER_BYTES;
	ctx->sample_rate = DTMF_AUDIO_SRATE;
	ctx->dtmf_l_seq = default_dtmf_l_seq;
	ctx->dtmf_r_seq = default_dtmf_r_seq;

	ctx->dtmf_seq_idx = 0;
	ctx->phase = 0;

	sai_setup(ctx);

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, DTMF_AUDIO_SRATE, DTMF_AUDIO_BITWIDTH);

	err = os_sem_init(&ctx->tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	memset(ctx->audio_buf, 0, ctx->audio_buf_size);

	os_printf("Playing following DTMF sequence (Sample Rate: %d Hz, Bit Width %d bits)\r\n", ctx->sample_rate, DTMF_AUDIO_BITWIDTH);
	os_printf("\tleft channel:  %s\r\n", ctx->dtmf_l_seq);
	os_printf("\tright channel: %s\r\n", ctx->dtmf_r_seq);

	return ctx;
}

static void play_dtmf_exit(void *handle)
{
	struct dtmf_ctx *ctx = handle;

	sai_drv_exit(&ctx->dev);

	codec_close();

	os_free(ctx);

	os_printf("End.\r\n");
}

void play_dtmf_task(void *parameters)
{
	void *handle;

	handle = play_dtmf_init(parameters);

	play_dtmf_run(handle);

	play_dtmf_exit(handle);
}
