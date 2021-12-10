/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "music.h"
#include "os/assert.h"
#include "os/semaphore.h"
#include "os/unistd.h"
#include "os/stdlib.h"
#include "sai_drv.h"
#include "sai_codec_config.h"

struct music_ctx {
	os_sem_t tx_semaphore;
	os_sem_t rx_semaphore;
	struct sai_device dev;
	uint32_t play_times;
};

static void rx_callback(uint8_t status, void *userData)
{
	struct music_ctx *ctx = userData;

	if (status == SAI_STATUS_NO_ERROR)
		os_sem_give(&ctx->rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void tx_callback(uint8_t status, void *userData)
{
	struct music_ctx *ctx = userData;

	if (status == SAI_STATUS_NO_ERROR)
		os_sem_give(&ctx->tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static int play_music_run(void *handle)
{
	struct music_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	int err;
	uintptr_t addr = (uintptr_t) music;

	while (1) {
		os_printf("play the music: %d times\r", ctx->play_times++);
		err = sai_write(dev, (uint8_t *)addr, MUSIC_LEN);
		if (!err) {
			err = os_sem_take(&ctx->tx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
		}

		os_msleep(2000);
	}

	return 0;
}

static void sai_setup(struct music_ctx *ctx)
{
	struct sai_cfg sai_config;

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = DEMO_AUDIO_BIT_WIDTH;
	sai_config.sample_rate = DEMO_AUDIO_SAMPLE_RATE;
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

static void *play_music_init(void *parameters)
{
	struct music_ctx *ctx;
	int err;

	ctx = os_malloc(sizeof(struct music_ctx));
	os_assert(ctx, "Playing DTMF failed with memory allocation error");

	sai_setup(ctx);

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH);

	err = os_sem_init(&ctx->tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	ctx->play_times = 1;

	return ctx;
}

static void play_music_exit(void *handle)
{
	struct music_ctx *ctx = handle;

	sai_drv_exit(&ctx->dev);

	codec_close();

	os_free(ctx);
}

void play_music_task(void *parameters)
{
	void *handle;

	handle = play_music_init(parameters);

	play_music_run(handle);

	play_music_exit(handle);
}
