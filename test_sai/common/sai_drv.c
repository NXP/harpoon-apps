/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sai.h"

#include "board.h"
#include "irq.h"
#include "os.h"
#include "os/assert.h"
#include "os/semaphore.h"
#include "os/unistd.h"
#include "sai_drv.h"

#ifdef PLAT_WITH_AUDIOMIX
#include "fsl_audiomix.h"
#endif

struct sai_dev_data {
	void *base;
	sai_handle_t *tx_handle;
	sai_handle_t *rx_handle;
};

struct drv_cb_user_data {
	void *dev;
	sai_callback_t app_callback;
	void *app_user_data;
};

sai_master_clock_t mclkConfig;
sai_handle_t tx_handle = {0}, rx_handle = {0};

static I2S_Type *const s_saiBases[] = I2S_BASE_PTRS;

static const IRQn_Type s_saiTxIRQ[] = I2S_TX_IRQS;

static struct sai_dev_data dev_data;

struct drv_cb_user_data rx_cb_user_data;
struct drv_cb_user_data tx_cb_user_data;


int sai_read(struct sai_device *dev, uint8_t *addr, size_t len)
{
	sai_transfer_t xfer;
	status_t ret;

	xfer.data = addr;
	xfer.dataSize = len;
	ret = SAI_TransferReceiveNonBlocking(dev->sai_base, dev->sai_rx_handle, &xfer);

	return (ret == kStatus_Success) ? 0 : -1;
}

int sai_write(struct sai_device *dev, uint8_t *addr, size_t len)
{
	sai_transfer_t xfer;
	status_t ret;

	xfer.data     = addr;
	xfer.dataSize = len;
	ret = SAI_TransferSendNonBlocking(dev->sai_base, dev->sai_tx_handle, &xfer);

	return (ret == kStatus_Success) ? 0 : -1;
}

static void sai_master_clock_config(struct sai_cfg *sai_config)
{
	I2S_Type *sai = (I2S_Type *)sai_config->sai_base;

	mclkConfig.mclkOutputEnable = true;
	mclkConfig.mclkHz           = sai_config->source_clock_hz;
	mclkConfig.mclkSourceClkHz  = sai_config->source_clock_hz;
	SAI_SetMasterClockConfig(sai, &mclkConfig);
}

static void sai_irq_handler(void *sai_dev_data)
{
	 struct sai_dev_data *sai_data = sai_dev_data;
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
		status_t status, void *user_data)
{
	struct drv_cb_user_data *drv_user_data = (struct drv_cb_user_data *)user_data;

	if (kStatus_SAI_RxError == status) {
		/* Handle the error. */
	} else {
		if (drv_user_data->app_callback)
			drv_user_data->app_callback(drv_user_data->dev, drv_user_data->app_user_data);
	}
}

static void tx_callback(I2S_Type *base, sai_handle_t *handle,
		status_t status, void *user_data)
{
	struct drv_cb_user_data *drv_user_data = (struct drv_cb_user_data *)user_data;

	if (kStatus_SAI_TxError == status) {
		/* Handle the error. */
	} else {
		if (drv_user_data->app_callback)
			drv_user_data->app_callback(drv_user_data->dev, drv_user_data->app_user_data);
	}
}

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

int sai_drv_setup(struct sai_device *dev, struct sai_cfg *sai_config)
{
	sai_transceiver_t config;
	IRQn_Type sai_irq_n;
	I2S_Type *sai = (I2S_Type *)sai_config->sai_base;
	uint32_t sai_id;
	int ret;

	sai_id = get_sai_id(sai);

	/* SAI init */
	SAI_Init(sai);

	/* I2S mode configurations */
	SAI_GetClassicI2SConfig(&config, sai_config->bit_width, kSAI_Stereo,
			1U << DEMO_SAI_CHANNEL);
	config.syncMode    = sai_config->tx_sync_mode;
	config.masterSlave = DEMO_SAI_MASTER_SLAVE;
	SAI_TransferTxSetConfig(sai, &tx_handle, &config);
	config.syncMode = sai_config->rx_sync_mode;
	SAI_TransferRxSetConfig(sai, &rx_handle, &config);

#ifdef PLAT_WITH_AUDIOMIX
	/* SAI bit clock source */
#ifdef CODEC_WM8960
	AUDIOMIX_AttachClk(AUDIOMIX, kAUDIOMIX_Attach_SAI3_MCLK1_To_SAI3_ROOT);
#elif defined(CODEC_HIFIBERRY)
	AUDIOMIX_AttachClk(AUDIOMIX, kAUDIOMIX_Attach_SAI5_MCLK1_To_SAI5_ROOT);
#endif
#endif
	/* set bit clock divider */
	SAI_TxSetBitClockRate(sai, sai_config->source_clock_hz,
			sai_config->sample_rate, sai_config->bit_width,
			sai_config->chan_numbers);
	SAI_RxSetBitClockRate(sai, sai_config->source_clock_hz,
			sai_config->sample_rate, sai_config->bit_width,
			sai_config->chan_numbers);

	/* master clock configurations */
	sai_master_clock_config(sai_config);

	tx_cb_user_data.dev = &dev;
	tx_cb_user_data.app_callback = sai_config->tx_callback;
	tx_cb_user_data.app_user_data = sai_config->tx_user_data;
	rx_cb_user_data.dev = &dev;
	rx_cb_user_data.app_callback = sai_config->rx_callback;
	rx_cb_user_data.app_user_data = sai_config->rx_user_data;

	tx_handle.base = sai;
	tx_handle.callback = tx_callback;
	tx_handle.userData = (void *)&tx_cb_user_data;
	rx_handle.base = sai;
	rx_handle.callback = rx_callback;
	rx_handle.userData = (void *)&rx_cb_user_data;

	dev_data.base = sai;
	dev_data.tx_handle = &tx_handle;
	dev_data.rx_handle = &rx_handle;

	/* Currently rx and tx use the same irq number */
	sai_irq_n = s_saiTxIRQ[sai_id];

	ret = irq_register(sai_irq_n, sai_irq_handler, &dev_data);
	os_assert(!ret, "Failed to register SAI IRQ! (%d)", ret);

	GIC_EnableIRQ(sai_irq_n);

	dev->sai_base = sai_config->sai_base;
	dev->sai_rx_handle = &rx_handle;
	dev->sai_tx_handle = &tx_handle;

	return 0;
}
