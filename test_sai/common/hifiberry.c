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

#define FRAME_BYTES	(DEMO_AUDIO_DATA_CHANNEL * DEMO_AUDIO_BIT_WIDTH / 8)
#define REC_PERIOD_SIZE		128
#define REC_PERIOD_BYTES	(REC_PERIOD_SIZE * FRAME_BYTES)
#define BUFFER_SIZE		REC_PERIOD_BYTES
#define BUFFER_NUMBER		(4U)

#define SAI_RX_PRIORITY		(configMAX_PRIORITIES - 1)
#define SAI_TX_PRIORITY		(configMAX_PRIORITIES - 1)

volatile uint32_t emptyBlock = BUFFER_NUMBER;

static uint8_t Buffer[BUFFER_NUMBER * BUFFER_SIZE];
static uint32_t tx_index = 0U, rx_index = 0U;
/*
 * Each buffer have a pair ofsemaphore
 * -----------------------------------------------------------------------
 * | semaphore | init_status   |    rx_task         |     tx_task        |
 * -----------------------------------------------------------------------
 * |   rx_sem  |    taken      |        rx-->give   | take-->tx          |
 * |   tx_sem  |    give       | take-->rx          |        tx-->give   |
 * -----------------------------------------------------------------------
 */
static os_sem_t buffer_rx_sem[BUFFER_SIZE];
static os_sem_t buffer_tx_sem[BUFFER_SIZE];

/* callback semaphore */
static os_sem_t tx_semaphore;
static os_sem_t rx_semaphore;
/* task semaphore */
static os_sem_t tx_task_sem;
static os_sem_t rx_task_sem;

static void rx_callback(const void *dev, void *userData)
{
	os_sem_give(&rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void tx_callback(const void *dev, void *userData)
{
	os_sem_give(&tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void sai_rx(void *param)
{
	struct sai_device *dev = (struct sai_device *)param;
	int err;
	uint32_t record_times = 1;

	err = os_sem_take(&rx_task_sem, 0, OS_SEM_TIMEOUT_MAX);
	os_assert(!err, "Can't take the rx task control semaphore (err: %d)", err);

	do {
#ifdef DEBUG
		os_printf("record %d\n\r", record_times++);
#endif
		err = os_sem_take(&buffer_tx_sem[rx_index], 0,
				OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the buffer semaphore (err: %d)", err);

		err = sai_read(dev, (uint8_t *)Buffer + rx_index * BUFFER_SIZE,
				BUFFER_SIZE);
		if (!err) {
			err = os_sem_take(&rx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
			os_sem_give(&buffer_rx_sem[rx_index], 0);
			rx_index++;
		}
		if (rx_index == BUFFER_NUMBER)
			rx_index = 0U;
	} while (1);
}

static void sai_tx(void *param)
{
	struct sai_device *dev = (struct sai_device *)param;
	int err;
	uint32_t play_times = 1;

	err = os_sem_take(&tx_task_sem, 0, OS_SEM_TIMEOUT_MAX);
	os_assert(!err, "Can't take the rx task control semaphore (err: %d)", err);

	do {
#ifdef DEBUG
		os_printf("play %d\n\r", play_times++);
#endif
		err = os_sem_take(&buffer_rx_sem[tx_index], 0,
				OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the buffer semaphore (err: %d)", err);

		err = sai_write(dev, (uint8_t *)Buffer + tx_index * BUFFER_SIZE,
				BUFFER_SIZE);
		if (!err) {
			err = os_sem_take(&tx_semaphore, 0,
					OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
			os_sem_give(&buffer_tx_sem[tx_index], 0);
			tx_index++;
		}
		if (tx_index == BUFFER_NUMBER)
			tx_index = 0U;
	} while (1);
}

static void sai_record_playback(struct sai_device *dev)
{
	BaseType_t xResult;
	int err, i;

	os_printf("HifiBerry record playback demo started\n\r");

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx interrupt semaphore initialization failed!");
	err = os_sem_init(&rx_semaphore, 0);
	os_assert(!err, "rx interrupt semaphore initialization failed!");
	err = os_sem_init(&tx_task_sem, 0);
	os_assert(!err, "tx task semaphore initialization failed!");
	err = os_sem_init(&rx_task_sem, 0);
	os_assert(!err, "rx task semaphore initialization failed!");
	for (i = 0; i < BUFFER_SIZE; i++) {
		err = os_sem_init(&buffer_tx_sem[i], 0);
		os_assert(!err, "buffer semaphore initialization failed!");
		err = os_sem_init(&buffer_rx_sem[i], 0);
		os_assert(!err, "buffer semaphore initialization failed!");
		err = os_sem_give(&buffer_tx_sem[i], 0);
		os_assert(!err, "give buffer semaphore failed!");
	}

	xResult = xTaskCreate(sai_rx, "sai_record_playback_task",
			configMINIMAL_STACK_SIZE + 100, (void *)dev,
			SAI_RX_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "Created sai test task failed");
	xResult = xTaskCreate(sai_tx, "sai_record_playback_task",
			configMINIMAL_STACK_SIZE + 100, (void *)dev,
			SAI_TX_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "Created sai test task failed");

	/* Release the rx task firstly, and then tx task */
	err = os_sem_give(&rx_task_sem, 0);
	os_assert(!err, "Can't give the rx task semaphore (err: %d)", err);
	err = os_sem_give(&tx_task_sem, 0);
	os_assert(!err, "Can't give the rx task semaphore (err: %d)", err);
}

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

	sai_record_playback(&dev);

	for (;;)
		;
}
