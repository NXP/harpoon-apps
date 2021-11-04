/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef PLAT_WITH_AUDIOMIX
#include "fsl_audiomix.h"
#endif
#include "fsl_sai.h"

#include "board.h"
#include "irq.h"
#include "os.h"
#include "os/assert.h"
#include "os/semaphore.h"
#include "os/unistd.h"

struct sai_dev_data {
	void *base;
	sai_handle_t *tx_handle;
	sai_handle_t *rx_handle;
};

sai_master_clock_t mclkConfig;
sai_handle_t txHandle = {0}, rxHandle = {0};

#ifndef SAI_PLAY_MUSIC

#define BUFFER_SIZE		(1024U)
#define BUFFER_NUMBER		(4U)

volatile uint32_t emptyBlock = BUFFER_NUMBER;

static uint8_t Buffer[BUFFER_NUMBER * BUFFER_SIZE];
static uint32_t tx_index = 0U, rx_index = 0U;

#endif /* SAI_PLAY_MUSIC */


static I2S_Type *const s_saiBases[] = I2S_BASE_PTRS;

static const IRQn_Type s_saiTxIRQ[] = I2S_TX_IRQS;

static os_sem_t tx_semaphore;
static os_sem_t rx_semaphore;

static struct sai_dev_data dev_data;

static void BOARD_MASTER_CLOCK_CONFIG(void)
{
	mclkConfig.mclkOutputEnable = true;
	mclkConfig.mclkHz           = DEMO_AUDIO_MASTER_CLOCK;
	mclkConfig.mclkSourceClkHz  = DEMO_SAI_CLK_FREQ;
	SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}

static void sai_irq_handler(void *data)
{
	 struct sai_dev_data *sai_data = data;
	 I2S_Type *base = sai_data->base;
	 uint32_t reg_rcsr = base->RCSR;
	 uint32_t reg_tcsr = base->TCSR;

#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
	if ((reg_rcsr & (I2S_RCSR_FRIE_MASK | I2S_RCSR_FEIE_MASK))
			&& (reg_rcsr & (I2S_RCSR_FRF_MASK | I2S_RCSR_FEF_MASK)))
#else
	if ((reg_rcsr & (I2S_RCSR_FWIE_MASK | I2S_RCSR_FEIE_MASK))
			&& (reg_rcsr & (I2S_RCSR_FWF_MASK | I2S_RCSR_FEF_MASK)))
#endif
	{
		SAI_TransferRxHandleIRQ(base, sai_data->rx_handle);
	}
#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
	if ((reg_tcsr & (I2S_TCSR_FRIE_MASK | I2S_TCSR_FEIE_MASK))
			&& (reg_tcsr & (I2S_TCSR_FRF_MASK | I2S_TCSR_FEF_MASK)))
#else
	if ((reg_tcsr & (I2S_TCSR_FWIE_MASK | I2S_TCSR_FEIE_MASK))
			&& (reg_tcsr & (I2S_TCSR_FWF_MASK | I2S_TCSR_FEF_MASK)))
#endif
	{
		SAI_TransferTxHandleIRQ(base, sai_data->tx_handle);
	}
}

static void rx_callback(I2S_Type *base, sai_handle_t *handle,
		status_t status, void *userData)
{
	if (kStatus_SAI_RxError == status) {
		/* Handle the error. */
	} else {
		os_sem_give(&rx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
#ifndef SAI_PLAY_MUSIC
		emptyBlock--;
#endif
	}
}

static void tx_callback(I2S_Type *base, sai_handle_t *handle,
		status_t status, void *userData)
{
	if (kStatus_SAI_TxError == status) {
		/* Handle the error. */
	} else {
		os_sem_give(&tx_semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
#ifndef SAI_PLAY_MUSIC
		emptyBlock++;
#endif
	}
}

#ifndef SAI_PLAY_MUSIC
static void record_playback(void)
{
	sai_transfer_t xfer;
	int err;
	uint32_t play_times = 1;

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");
	err = os_sem_init(&rx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	while (1) {
		os_printf("record %d\n\r", play_times);
		if (emptyBlock > 0) {
			xfer.data     = Buffer + rx_index * BUFFER_SIZE;
			xfer.dataSize = BUFFER_SIZE;
			if (kStatus_Success ==
					SAI_TransferReceiveNonBlocking(DEMO_SAI,
						&rxHandle, &xfer)) {
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
			xfer.data     = Buffer + tx_index * BUFFER_SIZE;
			xfer.dataSize = BUFFER_SIZE;
			if (kStatus_Success ==
					SAI_TransferSendNonBlocking(DEMO_SAI,
						&txHandle, &xfer)) {
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
static void play_music(void)
{
	sai_transfer_t xfer;
	int err;
	uint32_t play_times = 1;
	status_t ret;
	uintptr_t addr = (uintptr_t) music;

	err = os_sem_init(&tx_semaphore, 0);
	os_assert(!err, "tx semaphore initialization failed!");

	while (1) {
		os_printf("play the music: %d times\n\r", play_times++);
		xfer.data = (uint8_t *)addr;
		xfer.dataSize = MUSIC_LEN;
		ret = SAI_TransferSendNonBlocking(DEMO_SAI, &txHandle, &xfer);
		if (ret == kStatus_Success) {
			err = os_sem_take(&tx_semaphore, 0, OS_SEM_TIMEOUT_MAX);
			os_assert(!err, "Can't take the tx semaphore (err: %d)", err);
		}

		os_msleep(2000);
	}
}

#endif /* SAI_PLAY_MUSIC */

uint32_t get_sai_id(I2S_Type *base)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(s_saiBases); i++) {
		if (s_saiBases[i] == base)
			return i;
	}

	/* Can't find SAI instance, hang here */
	os_assert(false, "Can't find SAI instance (%x)", (uintptr_t)base);
}

void sai_setup(uint8_t sai_id)
{
	sai_transceiver_t config;
	IRQn_Type sai_irqn;
	int ret;

	/* SAI init */
	SAI_Init(DEMO_SAI);

	/* I2S mode configurations */
	SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo,
			1U << DEMO_SAI_CHANNEL);
	config.syncMode    = DEMO_SAI_TX_SYNC_MODE;
	config.masterSlave = DEMO_SAI_MASTER_SLAVE;
	SAI_TransferTxSetConfig(DEMO_SAI, &txHandle, &config);
	config.syncMode = DEMO_SAI_RX_SYNC_MODE;
	SAI_TransferRxSetConfig(DEMO_SAI, &rxHandle, &config);

#ifdef PLAT_WITH_AUDIOMIX
	/* SAI bit clock source */
	AUDIOMIX_AttachClk(AUDIOMIX, kAUDIOMIX_Attach_SAI3_MCLK1_To_SAI3_ROOT);
#endif
	/* set bit clock divider */
	SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK,
			DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
			DEMO_AUDIO_DATA_CHANNEL);
	SAI_RxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK,
			DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

	/* master clock configurations */
	BOARD_MASTER_CLOCK_CONFIG();

	txHandle.base = DEMO_SAI;
	txHandle.callback = tx_callback;
	rxHandle.base = DEMO_SAI;
	rxHandle.callback = rx_callback;

	dev_data.base = DEMO_SAI;
	dev_data.tx_handle = &txHandle;
	dev_data.rx_handle = &rxHandle;

	/* Currently rx and tx use the same irq number */
	sai_irqn = s_saiTxIRQ[sai_id];

	ret = irq_register(sai_irqn, sai_irq_handler, &dev_data);
	os_assert(!ret, "Failed to register SAI IRQ! (%d)", ret);

	GIC_EnableIRQ(sai_irqn);
}

void sai_test_task(void *parameters)
{
#ifndef SAI_PLAY_MUSIC
	record_playback();
#else
	play_music();
#endif

	for (;;)
		;
}
