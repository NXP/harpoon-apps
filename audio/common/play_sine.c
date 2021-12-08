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
#include "sine_wave.h"
#include "sai_codec_config.h"

#define PLAY_AUDIO_SRATE	44100 /* default sampling rate */
#define PLAY_AUDIO_CHANNELS	2
#define PLAY_AUDIO_BITWIDTH	32

struct sine_ctx {
	/* callback semaphore */
	os_sem_t tx_semaphore;
	os_sem_t rx_semaphore;

	struct sai_device dev;
};

static void rx_callback(const void *dev, void *userData)
{
	struct sine_ctx *ctx = userData;

	os_sem_give(&ctx->rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void tx_callback(const void *dev, void *userData)
{
	struct sine_ctx *ctx = userData;

	os_sem_give(&ctx->tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static int play_sine_run(void *handle)
{
	struct sine_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	uintptr_t addr = (uintptr_t)sine_wave;
	size_t len = sizeof(sine_wave);
	int err;

	while (1) {
		err = sai_write(dev, (uint8_t *)addr, len);
		if (!err) {
			err = os_sem_take(&ctx->tx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
		}
	}

	return 0;
}

static void sai_setup(struct sine_ctx *ctx)
{
	struct sai_cfg sai_config;

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = PLAY_AUDIO_BITWIDTH;
	sai_config.sample_rate = PLAY_AUDIO_SRATE;
	sai_config.chan_numbers = PLAY_AUDIO_CHANNELS;
	sai_config.source_clock_hz = DEMO_AUDIO_MASTER_CLOCK;
	sai_config.tx_sync_mode = DEMO_SAI_TX_SYNC_MODE;
	sai_config.rx_sync_mode = DEMO_SAI_RX_SYNC_MODE;
	sai_config.rx_callback = rx_callback;
	sai_config.rx_user_data = ctx;
	sai_config.tx_callback = tx_callback;
	sai_config.tx_user_data = ctx;

	sai_drv_setup(&ctx->dev, &sai_config);
}

static void *play_sine_init(void *parameters)
{
	struct sine_ctx *ctx;
	int err;

	ctx = os_malloc(sizeof(struct sine_ctx));
	os_assert(ctx, "Playing DTMF failed with memory allocation error");

	sai_setup(ctx);

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, PLAY_AUDIO_SRATE, PLAY_AUDIO_BITWIDTH);

	os_printf("HifiBerry playing Sine wave is started (Sample Rate: %d Hz, Bit Width: %d bits)\r\n",
			PLAY_AUDIO_SRATE, PLAY_AUDIO_BITWIDTH);
	err = os_sem_init(&ctx->tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	return ctx;
}

static void play_sine_exit(void *handle)
{
	struct sine_ctx *ctx = handle;

	/* Stop SAI */

	codec_close();

	os_free(ctx);
}

void play_sine_task(void *parameters)
{
	void *handle;

	handle = play_sine_init(parameters);

	play_sine_run(handle);

	play_sine_exit(handle);
}
