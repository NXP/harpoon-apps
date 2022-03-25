/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "log.h"
#include "music.h"
#include "os/assert.h"
#include "os/stdlib.h"
#include "sai_drv.h"
#include "sai_codec_config.h"
#include "audio.h"

#define PLAY_AUDIO_SRATE	44100 /* default sampling rate */
#define PLAY_AUDIO_CHANNELS	2
#define PLAY_AUDIO_BITWIDTH	16

struct music_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;

	struct sai_device dev;
	uint32_t play_times;
};

void play_music_stats(void *handle)
{
	struct music_ctx *ctx = handle;

	log_info("Played Music: %u times", ctx->play_times);
}

static void tx_callback(uint8_t status, void *userData)
{
	struct music_ctx *ctx = userData;

	ctx->event_send(ctx->event_data, status);
}

int play_music_run(void *handle, struct event *e)
{
	struct music_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	int err;
	uintptr_t addr = (uintptr_t) music;

	err = sai_write(dev, (uint8_t *)addr, MUSIC_LEN);

	ctx->play_times++;

	return err;
}

static void sai_setup(struct music_ctx *ctx)
{
	struct sai_cfg sai_config = {0};

	sai_config.sai_base = (void *)DEMO_SAI;
	sai_config.bit_width = PLAY_AUDIO_BITWIDTH;
	sai_config.sample_rate = PLAY_AUDIO_SRATE;
	sai_config.chan_numbers = PLAY_AUDIO_CHANNELS;
	sai_config.source_clock_hz = DEMO_AUDIO_MASTER_CLOCK;
	sai_config.tx_sync_mode = DEMO_SAI_TX_SYNC_MODE;
	sai_config.rx_sync_mode = DEMO_SAI_RX_SYNC_MODE;
	sai_config.tx_callback = tx_callback;
	sai_config.tx_user_data = ctx;
	sai_config.masterSlave = DEMO_SAI_MASTER_SLAVE;

	sai_drv_setup(&ctx->dev, &sai_config);
}

void *play_music_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	struct music_ctx *ctx;
	enum codec_id cid;

	ctx = os_malloc(sizeof(struct music_ctx));
	os_assert(ctx, "Playing Music failed with memory allocation error");

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	sai_setup(ctx);

	cid = DEMO_CODEC_ID;
	codec_setup(cid);
	codec_set_format(cid, DEMO_AUDIO_MASTER_CLOCK, PLAY_AUDIO_SRATE, PLAY_AUDIO_BITWIDTH);

	log_info("Playing Music (Sample Rate: %d Hz, Bit Width: %d bits)\n", PLAY_AUDIO_SRATE, PLAY_AUDIO_BITWIDTH);

	ctx->play_times = 1;

	return ctx;
}

void play_music_exit(void *handle)
{
	struct music_ctx *ctx = handle;
	enum codec_id cid;

	sai_drv_exit(&ctx->dev);

	cid = DEMO_CODEC_ID;
	codec_close(cid);

	os_free(ctx);

	log_info("\nEnd.\n");
}
