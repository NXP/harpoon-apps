/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "os/assert.h"
#include "os/semaphore.h"
#include "os/unistd.h"
#include "sai_drv.h"

#ifndef SAI_PLAY_MUSIC

#define BUFFER_SIZE		(1024U)
#define BUFFER_NUMBER		(4U)

volatile uint32_t emptyBlock = BUFFER_NUMBER;

static uint8_t Buffer[BUFFER_NUMBER * BUFFER_SIZE];
static uint32_t tx_index = 0U, rx_index = 0U;

#endif /* SAI_PLAY_MUSIC */


static os_sem_t tx_semaphore;
static os_sem_t rx_semaphore;


static void rx_callback(const void *dev, void *userData)
{
	os_sem_give(&rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
#ifndef SAI_PLAY_MUSIC
	emptyBlock--;
#endif
}

static void tx_callback(const void *dev, void *userData)
{
	os_sem_give(&tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
#ifndef SAI_PLAY_MUSIC
	emptyBlock++;
#endif
}

#ifndef SAI_PLAY_MUSIC
static void record_playback(struct sai_device *dev)
{
	int err;
	uint32_t play_times = 1;

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");
	err = os_sem_init(&rx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	while (1) {
		os_printf("record %d\n\r", play_times);
		if (emptyBlock > 0) {
			err = sai_read(dev, (uint8_t *)Buffer + rx_index * BUFFER_SIZE, BUFFER_SIZE);
			if (!err) {
				rx_index++;
				err = os_sem_take(&rx_semaphore, 0,
						OS_SEM_TIMEOUT_MAX);
				os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
			}
			if (rx_index == BUFFER_NUMBER)
			{
				rx_index = 0U;
			}
		}
		os_printf("play %d\n\r", play_times++);
		if (emptyBlock < BUFFER_NUMBER) {
			err = sai_write(dev, (uint8_t *)Buffer + tx_index * BUFFER_SIZE, BUFFER_SIZE);
			if (!err) {
				tx_index++;
				err = os_sem_take(&tx_semaphore, 0,
						OS_SEM_TIMEOUT_MAX);
				os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
			}
			if (tx_index == BUFFER_NUMBER) {
				tx_index = 0U;
			}
		}
	}
}

#else /* SAI_PLAY_MUSIC */

#include "music.h"
static void play_music(struct sai_device *dev)
{
	int err;
	uint32_t play_times = 1;
	uintptr_t addr = (uintptr_t) music;

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	while (1) {
		os_printf("play the music: %d times\n\r", play_times++);
		err = sai_write(dev, (uint8_t *)addr, MUSIC_LEN);
		if (!err) {
			err = os_sem_take(&tx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
		}

		os_msleep(2000);
	}
}

#endif /* SAI_PLAY_MUSIC */

void sai_setup(struct sai_device *dev)
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

void sai_test_task(void *parameters)
{
	struct sai_device dev;

	sai_setup(&dev);

#ifndef SAI_PLAY_MUSIC
	record_playback(&dev);
#else
	play_music(&dev);
#endif

	for (;;)
		;
}
