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
#include "sai_drv.h"
#include "sai_codec_config.h"

static os_sem_t tx_semaphore;
static os_sem_t rx_semaphore;


static void rx_callback(const void *dev, void *userData)
{
	os_sem_give(&rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void tx_callback(const void *dev, void *userData)
{
	os_sem_give(&tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void play_music(struct sai_device *dev)
{
	int err;
	uint32_t play_times = 1;
	uintptr_t addr = (uintptr_t) music;

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	while (1) {
		os_printf("play the music: %d times\r", play_times++);
		err = sai_write(dev, (uint8_t *)addr, MUSIC_LEN);
		if (!err) {
			err = os_sem_take(&tx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
		}

		os_msleep(2000);
	}
}

static void sai_setup(struct sai_device *dev)
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
	sai_config.tx_callback = tx_callback;

	sai_drv_setup(dev, &sai_config);
}

void play_music_task(void *parameters)
{
	struct sai_device dev;

	sai_setup(&dev);

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH);

	play_music(&dev);

	for (;;)
		;
}
