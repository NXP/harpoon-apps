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
#include "os/unistd.h"
#include "sai_drv.h"
#include "dtmf_wave.h"

#define DTMF_AUDIO_SRATE 44100 /* default sampling rate */
#define DTMF_AUDIO_BITWIDTH 32
#define DTMF_TONE_DURATION_MS 120

#define BUFFER_BYTES (DTMF_AUDIO_SRATE * 2 * (DTMF_AUDIO_BITWIDTH / 8) \
		* DTMF_TONE_DURATION_MS / 1000)

static uint32_t *audio_buf;

/* callback semaphore */
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

void play_dtmf(struct sai_device *dev)
{
	const char default_dtmf_l_seq[] = "1123ABCD0123456789*#";
	const char default_dtmf_r_seq[] = "#*9876543210DCBA3211";
	const char *dtmf_l_seq = default_dtmf_l_seq;
	const char *dtmf_r_seq = default_dtmf_r_seq;
	int sample_rate = DTMF_AUDIO_SRATE;
	size_t audio_buf_size = BUFFER_BYTES;
	unsigned int dtmf_seq_idx = 0;
	uint32_t phase = 0;
	int err;

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	audio_buf = pvPortMalloc(audio_buf_size);
	os_assert(audio_buf, "Playing DTMF failed with memory allocation error");
	memset(audio_buf, 0, audio_buf_size);

	os_printf("Playing following DTMF sequence at %d Hz:\r\n", sample_rate);
	os_printf("\tleft channel:  %s\r\n", dtmf_l_seq);
	os_printf("\tright channel: %s\r\n", dtmf_r_seq);

	while (dtmf_seq_idx < strlen(dtmf_l_seq)) {
		/* prepare dtmf audio buffer */
		generate_dtmf_tone(audio_buf, dtmf_l_seq[dtmf_seq_idx],
				dtmf_r_seq[dtmf_seq_idx], sample_rate,
				DTMF_TONE_DURATION_MS, &phase);
		dtmf_seq_idx++;

		/* transmit audio buffer */
		err = sai_write(dev, (uint8_t *)audio_buf, audio_buf_size);
		if (!err) {
			err = os_sem_take(&tx_semaphore, 0,
					OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)",
					err);
		}

		/* prepare blank buffer */
		memset(audio_buf, 0, audio_buf_size);

		/* transmit audio buffer */
		err = sai_write(dev, (uint8_t *)audio_buf, audio_buf_size);
		if (!err) {
			err = os_sem_take(&tx_semaphore, 0,
					OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)",
					err);
		}
	}
	vPortFree(audio_buf);
	os_printf("End.\r\n");
}

void sai_setup(struct sai_device *dev)
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
	sai_config.tx_callback = tx_callback;

	sai_drv_setup(dev, &sai_config);
}

void sai_test_task(void *parameters)
{
	struct sai_device dev;

	sai_setup(&dev);

	play_dtmf(&dev);

	for (;;)
		;
}
