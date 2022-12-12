/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "os/irq.h"
#include "os/assert.h"
#include "sai_drv.h"

#include "fsl_sai.h"

static I2S_Type *const s_saiBases[] = I2S_BASE_PTRS;

static const IRQn_Type s_saiTxIRQ[] = I2S_TX_IRQS;


static bool __sai_rx_is_sync(void *base)
{
	return ((((I2S_Type *)base)->RCR2 & I2S_RCR2_SYNC_MASK) != 0U);
}

static bool __sai_tx_is_sync(void *base)
{
	return ((((I2S_Type *)base)->TCR2 & I2S_TCR2_SYNC_MASK) != 0U);
}

static bool __sai_rx_is_enabled(void *base)
{
	return ((((I2S_Type *)base)->RCSR & I2S_RCSR_RE_MASK) != 0U);
}

static bool __sai_tx_is_enabled(void *base)
{
	return ((((I2S_Type *)base)->TCSR & I2S_TCSR_TE_MASK) != 0U);
}

static void __sai_rx_enable(void *base)
{
	((I2S_Type *)base)->RCSR = ((((I2S_Type *)base)->RCSR & 0xFFE3FFFFU) | I2S_RCSR_RE_MASK);
}

static void __sai_tx_enable(void *base)
{
	((I2S_Type *)base)->TCSR = ((((I2S_Type *)base)->TCSR & 0xFFE3FFFFU) | I2S_TCSR_TE_MASK);
}

static void __sai_rx_disable(void *base)
{
	((I2S_Type *)base)->RCSR = ((((I2S_Type *)base)->RCSR & 0xFFE3FFFFU) & (~I2S_RCSR_RE_MASK));
}

static void __sai_tx_disable(void *base)
{
	((I2S_Type *)base)->TCSR = ((((I2S_Type *)base)->TCSR & 0xFFE3FFFFU) & (~I2S_TCSR_TE_MASK));
}

void __sai_enable_rx(void *base, bool enable_irq)
{
	if (enable_irq)
		__sai_enable_irq(base, true, false);

	/* For sync Rx/Tx, enable is done in Tx enable */
	if (!__sai_tx_is_sync(base) && !__sai_rx_is_sync(base))
		__sai_rx_enable(base);
}

void __sai_enable_tx(void *base, bool enable_irq)
{
	if (enable_irq)
		__sai_enable_irq(base, false, true);

	if (__sai_tx_is_sync(base)) {
		__sai_tx_enable(base);
		__sai_rx_enable(base);
	} else if (__sai_rx_is_sync(base)) {
		__sai_rx_enable(base);
		__sai_tx_enable(base);
	} else {
		__sai_tx_enable(base);
	}
}

void __sai_disable_rx(void *base)
{
	/* For sync Rx, disable is done in Tx disable */
	if (__sai_rx_is_sync(base))
		return;

	__sai_rx_disable(base);

	/* Wait for completion */
	while (__sai_rx_is_enabled(base))
		;

	SAI_RxClearStatusFlags(base, I2S_RCSR_FEF_MASK);

	/* Reset FIFO */
	SAI_RxSoftwareReset(base, kSAI_ResetTypeFIFO);

	if (__sai_tx_is_sync(base)) {
		__sai_tx_disable(base);

		/* Need to reset Tx fifo after Rx is disabled */
		SAI_TxClearStatusFlags(base, I2S_TCSR_FEF_MASK);

		SAI_TxSoftwareReset(base, kSAI_ResetTypeFIFO);
	}
}

void __sai_disable_tx(void *base)
{
	/* For sync Tx, disable is done in Rx disable */
	if (__sai_tx_is_sync(base))
		return;

	__sai_tx_disable(base);

	/* Wait for completion */
	while (__sai_tx_is_enabled(base))
		;

	SAI_TxClearStatusFlags(base, I2S_TCSR_FEF_MASK);

	/* Reset FIFO */
	SAI_TxSoftwareReset(base, kSAI_ResetTypeFIFO);

	if (__sai_rx_is_sync(base)) {
		__sai_rx_disable(base);

		/* Need to reset Rx fifo after Tx is disabled */
		SAI_RxClearStatusFlags(base, I2S_RCSR_FEF_MASK);
		SAI_RxSoftwareReset(base, kSAI_ResetTypeFIFO);
	}
}

int sai_read(struct sai_device *dev, uint8_t *addr, size_t len)
{
	sai_transfer_t xfer;
	status_t ret;

	xfer.data = addr;
	xfer.dataSize = len;
	ret = SAI_TransferReceiveNonBlocking(dev->sai_base, &dev->sai_rx_handle,
			&xfer);

	return (ret == kStatus_Success) ? 0 : -1;
}

int sai_write(struct sai_device *dev, uint8_t *addr, size_t len)
{
	sai_transfer_t xfer;
	status_t ret;

	xfer.data     = addr;
	xfer.dataSize = len;
	ret = SAI_TransferSendNonBlocking(dev->sai_base, &dev->sai_tx_handle,
			&xfer);

	return (ret == kStatus_Success) ? 0 : -1;
}

void sai_enable_irq(struct sai_device *dev, bool rx_irq, bool tx_irq)
{
	__sai_enable_irq(dev->sai_base, rx_irq, tx_irq);
}

void sai_disable_irq(struct sai_device *dev, bool rx_irq, bool tx_irq)
{
	__sai_disable_irq(dev->sai_base, rx_irq, tx_irq);
}

void sai_enable_rx(struct sai_device *dev, bool enable_irq)
{
	__sai_enable_rx(dev->sai_base, enable_irq);
}

void sai_enable_tx(struct sai_device *dev, bool enable_irq)
{
	__sai_enable_tx(dev->sai_base, enable_irq);
}

void sai_disable_rx(struct sai_device *dev)
{
	__sai_disable_rx(dev->sai_base);
}

void sai_disable_tx(struct sai_device *dev)
{
	__sai_disable_tx(dev->sai_base);
}

static void sai_master_clock_config(struct sai_cfg *sai_config)
{
	I2S_Type *sai = (I2S_Type *)sai_config->sai_base;
	sai_master_clock_t mclkConfig;

	mclkConfig.mclkOutputEnable = true;
	mclkConfig.mclkHz           = sai_config->source_clock_hz;
	mclkConfig.mclkSourceClkHz  = sai_config->source_clock_hz;
	SAI_SetMasterClockConfig(sai, &mclkConfig);
}

static void sai_irq_handler_continuous(void *data)
{
	struct sai_device *dev = data;

	dev->rx_callback(SAI_STATUS_NO_ERROR, dev->rx_user_data);
}

static void sai_irq_handler(void *data)
{
	struct sai_device *dev = data;
	I2S_Type *base = dev->sai_base;
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
		SAI_TransferRxHandleIRQ(base, &dev->sai_rx_handle);
	}
#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
	if ((reg_tcsr & (I2S_TCSR_FRIE_MASK | I2S_TCSR_FEIE_MASK))
			&& (reg_tcsr & (I2S_TCSR_FRF_MASK | I2S_TCSR_FEF_MASK)))
#else
	if ((reg_tcsr & (I2S_TCSR_FWIE_MASK | I2S_TCSR_FEIE_MASK))
			&& (reg_tcsr & (I2S_TCSR_FWF_MASK | I2S_TCSR_FEF_MASK)))
#endif
	{
		SAI_TransferTxHandleIRQ(base, &dev->sai_tx_handle);
	}
}

static void rx_callback(I2S_Type *base, sai_handle_t *handle,
		status_t status, void *user_data)
{
	struct sai_device *dev = (struct sai_device *)user_data;
	uint8_t sai_status;

	sai_status = (status == kStatus_SAI_RxError) ?
		SAI_STATUS_RX_FF_ERR: SAI_STATUS_NO_ERROR;

	if (dev->rx_callback)
		dev->rx_callback(sai_status, dev->rx_user_data);
}

static void tx_callback(I2S_Type *base, sai_handle_t *handle,
		status_t status, void *user_data)
{
	struct sai_device *dev = (struct sai_device *)user_data;
	uint8_t sai_status;

	sai_status = (status == kStatus_SAI_TxError) ?
		SAI_STATUS_TX_FF_ERR : SAI_STATUS_NO_ERROR;

	if (dev->tx_callback)
		dev->tx_callback(sai_status, dev->tx_user_data);
}

void *__sai_base(uint32_t id)
{
	if (id >= ARRAY_SIZE(s_saiBases))
		return NULL;

	return s_saiBases[id];
}

uint32_t get_sai_id(I2S_Type *base)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(s_saiBases); i++) {
		if (s_saiBases[i] == base)
			break;
	}

	if (i >= ARRAY_SIZE(s_saiBases)) {
		/* Can't find SAI instance, hang here */
		os_assert(false, "Can't find SAI instance (%p)", base);
	}

	return i;
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
	config.syncMode            = sai_config->tx_sync_mode;
	config.masterSlave         = sai_config->masterSlave;
	config.bitClock.bclkSource = sai_config->msel;

	if (sai_config->fifo_water_mark)
		config.fifo.fifoWatermark = sai_config->fifo_water_mark;

	SAI_TransferTxSetConfig(sai, &dev->sai_tx_handle, &config);
	config.syncMode = sai_config->rx_sync_mode;
	SAI_TransferRxSetConfig(sai, &dev->sai_rx_handle, &config);

	/* Set FIFO to continue from next frame on error */
	SAI_TxSetFIFOErrorContinue(sai, false);
	SAI_RxSetFIFOErrorContinue(sai, false);

	/* set bit clock divider */
	SAI_TxSetBitClockRate(sai, sai_config->source_clock_hz,
			sai_config->sample_rate, sai_config->bit_width,
			sai_config->chan_numbers);
	SAI_RxSetBitClockRate(sai, sai_config->source_clock_hz,
			sai_config->sample_rate, sai_config->bit_width,
			sai_config->chan_numbers);

	/* master clock configurations */
	sai_master_clock_config(sai_config);

	dev->tx_callback = sai_config->tx_callback;
	dev->tx_user_data = sai_config->tx_user_data;

	dev->rx_callback = sai_config->rx_callback;
	dev->rx_user_data = sai_config->rx_user_data;

	dev->sai_tx_handle.base = sai;
	dev->sai_tx_handle.callback = tx_callback;
	dev->sai_tx_handle.userData = (void *)dev;
	dev->sai_rx_handle.base = sai;
	dev->sai_rx_handle.callback = rx_callback;
	dev->sai_rx_handle.userData = (void *)dev;

	dev->sai_base = sai_config->sai_base;

	/* Currently rx and tx use the same irq number */
	sai_irq_n = s_saiTxIRQ[sai_id];

	switch (sai_config->working_mode) {
		case SAI_RX_IRQ_MODE:
			ret = os_irq_register(sai_irq_n, sai_irq_handler_continuous, dev, OS_IRQ_PRIO_DEFAULT);
			os_assert(!ret, "Failed to register SAI IRQ! (%d)", ret);
			os_irq_enable(sai_irq_n);
			break;
		case SAI_CALLBACK_MODE:
			ret = os_irq_register(sai_irq_n, sai_irq_handler, dev, OS_IRQ_PRIO_DEFAULT);
			os_assert(!ret, "Failed to register SAI IRQ! (%d)", ret);
			os_irq_enable(sai_irq_n);
			break;
		case SAI_POLLING_MODE:
		default:
			break;
	}

	return 0;
}

void sai_drv_exit(struct sai_device *dev)
{
	I2S_Type *sai = (I2S_Type *)dev->sai_base;

	SAI_Deinit(sai);
}
