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
#include "sai_codec_config.h"

#define FRAME_BYTES	(DEMO_AUDIO_DATA_CHANNEL * DEMO_AUDIO_BIT_WIDTH / 8)
#define REC_PERIOD_SIZE		128
#define REC_PERIOD_BYTES	(REC_PERIOD_SIZE * FRAME_BYTES)
#define BUFFER_SIZE		REC_PERIOD_BYTES
#define BUFFER_NUMBER		(4U)

#define SAI_TX_PRIORITY		(configMAX_PRIORITIES - 1)

struct rec_play2_ctx {
	struct sai_device dev;

	volatile uint32_t emptyBlock;

	TaskHandle_t taskHandle;

	uint8_t Buffer[BUFFER_NUMBER * BUFFER_SIZE];
	uint32_t tx_index;
	uint32_t rx_index;
/*
 * Each buffer have a pair ofsemaphore
 * -----------------------------------------------------------------------
 * | semaphore | init_status   |    rx_task         |     tx_task        |
 * -----------------------------------------------------------------------
 * |   rx_sem  |    taken      |        rx-->give   | take-->tx          |
 * |   tx_sem  |    give       | take-->rx          |        tx-->give   |
 * -----------------------------------------------------------------------
 */
	os_sem_t buffer_rx_sem[BUFFER_SIZE];
	os_sem_t buffer_tx_sem[BUFFER_SIZE];

	/* callback semaphore */
	os_sem_t tx_semaphore;
	os_sem_t rx_semaphore;

	/* task semaphore */
	os_sem_t tx_task_sem;
};

static void rx_callback(uint8_t status, void *userData)
{
	struct rec_play2_ctx *ctx = userData;

	if (status == SAI_STATUS_NO_ERROR)
		os_sem_give(&ctx->rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

static void tx_callback(uint8_t status, void *userData)
{
	struct rec_play2_ctx *ctx = userData;

	if (status == SAI_STATUS_NO_ERROR)
		os_sem_give(&ctx->tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

int rec_play2_run(void *handle)
{
	struct rec_play2_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	int err;
#ifdef DEBUG
	uint32_t record_times = 1;
	uint32_t print_steps = 100;
	uint32_t next_print_times = record_times + print_steps;
#endif

	do {
		err = os_sem_take(&ctx->buffer_tx_sem[ctx->rx_index], 0,
				OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the buffer semaphore (err: %d)", err);

		err = sai_read(dev, (uint8_t *)ctx->Buffer + ctx->rx_index * BUFFER_SIZE,
				BUFFER_SIZE);
		if (!err) {
			err = os_sem_take(&ctx->rx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
			os_sem_give(&ctx->buffer_rx_sem[ctx->rx_index], 0);
			ctx->rx_index++;
#ifdef DEBUG
			record_times++;
#endif
		}
		if (ctx->rx_index == BUFFER_NUMBER)
			ctx->rx_index = 0U;
#ifdef DEBUG
		if (record_times == next_print_times) {
			os_printf("Record:   %08d \r\n", record_times);
			next_print_times = record_times + print_steps;
		}
#endif
	} while (1);
}

static void sai_tx(void *handle)
{
	struct rec_play2_ctx *ctx = handle;
	struct sai_device *dev = &ctx->dev;
	int err;
#ifdef DEBUG
	uint32_t play_times = 1;
	uint32_t print_steps = 100;
	uint32_t next_print_times = play_times + print_steps;
#endif

	err = os_sem_take(&ctx->tx_task_sem, 0, OS_SEM_TIMEOUT_MAX);
	os_assert(!err, "Can't take the tx task control semaphore (err: %d)", err);

	do {
		err = os_sem_take(&ctx->buffer_rx_sem[ctx->tx_index], 0,
				OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the buffer semaphore (err: %d)", err);

		err = sai_write(dev, (uint8_t *)ctx->Buffer + ctx->tx_index * BUFFER_SIZE,
				BUFFER_SIZE);
		if (!err) {
			err = os_sem_take(&ctx->tx_semaphore, 0,
					OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
			os_sem_give(&ctx->buffer_tx_sem[ctx->tx_index], 0);
			ctx->tx_index++;
#ifdef DEBUG
			play_times++;
#endif
		}
		if (ctx->tx_index == BUFFER_NUMBER)
			ctx->tx_index = 0U;
#ifdef DEBUG
		if (play_times == next_print_times) {
			os_printf("Playback: %08d \r\n", play_times);
			next_print_times = play_times + print_steps;
		}
#endif
	} while (1);
}

static void sai_record_playback(struct rec_play2_ctx *ctx)
{
	BaseType_t xResult;
	int err, i;

	os_printf("Record and playback (two threads) started (Sample Rate: %d Hz, Bit Width: %d bits)\r\n",
			DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH);

	err = os_sem_init(&ctx->tx_semaphore, 0);
	os_assert(!err, "tx interrupt semaphore initialization failed!");

	err = os_sem_init(&ctx->rx_semaphore, 0);
	os_assert(!err, "rx interrupt semaphore initialization failed!");

	err = os_sem_init(&ctx->tx_task_sem, 0);
	os_assert(!err, "tx task semaphore initialization failed!");

	for (i = 0; i < BUFFER_SIZE; i++) {
		err = os_sem_init(&ctx->buffer_tx_sem[i], 0);
		os_assert(!err, "buffer semaphore initialization failed!");
		err = os_sem_init(&ctx->buffer_rx_sem[i], 0);
		os_assert(!err, "buffer semaphore initialization failed!");
		err = os_sem_give(&ctx->buffer_tx_sem[i], 0);
		os_assert(!err, "give buffer semaphore failed!");
	}

	xResult = xTaskCreate(sai_tx, "sai_record_playback_task",
			configMINIMAL_STACK_SIZE + 100, (void *)ctx,
			SAI_TX_PRIORITY, &ctx->taskHandle);
	os_assert(xResult == pdPASS, "Created sai test task failed");

	/* Release the rx task firstly, and then tx task */
	err = os_sem_give(&ctx->tx_task_sem, 0);
	os_assert(!err, "Can't give the rx task semaphore (err: %d)", err);
}

static void sai_setup(struct rec_play2_ctx *ctx)
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

static void *rec_play2_init(void *parameters)
{
	struct rec_play2_ctx *ctx;

	ctx = os_malloc(sizeof(struct rec_play2_ctx));
	os_assert(ctx, "Record and playback failed with memory allocation error");

	sai_setup(ctx);

	ctx->emptyBlock = BUFFER_NUMBER;
	ctx->tx_index = 0U;
	ctx->rx_index = 0U;

	codec_setup();
	codec_set_format(DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH);

	sai_record_playback(ctx);

	return ctx;
}

static void rec_play2_exit(void *handle)
{
	struct rec_play2_ctx *ctx = handle;

	sai_drv_exit(&ctx->dev);

	codec_close();

	vTaskDelete(ctx->taskHandle);

	os_free(ctx);

	os_printf("End.\r\n");
}

void rec_play2_task(void *parameters)
{
	void *ctx;

	ctx = rec_play2_init(parameters);

	rec_play2_run(ctx);

	rec_play2_exit(ctx);

	for (;;)
		;
}
