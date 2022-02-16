/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SAI_DRV_H_
#define _SAI_DRV_H_

#include "fsl_sai.h"

/* SAI status checked in ISR */
#define	SAI_STATUS_NO_ERROR	0
#define	SAI_STATUS_TX_FF_ERR	(1 << 0)
#define	SAI_STATUS_RX_FF_ERR	(1 << 1)

typedef void (*sai_callback_t)(uint8_t status, void *user_data);

struct sai_device {
	void *sai_base;
	sai_handle_t sai_tx_handle;
	sai_handle_t sai_rx_handle;
	sai_callback_t rx_callback;
	void *rx_user_data;
	sai_callback_t tx_callback;
	void *tx_user_data;
};

enum sai_mode {
	SAI_NORMAL_MODE,
	SAI_CONTINUE_MODE
};

struct sai_cfg {
	void *sai_base;
	sai_callback_t rx_callback;
	sai_callback_t tx_callback;
	void *rx_user_data;
	void *tx_user_data;
	sai_word_width_t bit_width;
	sai_sample_rate_t sample_rate;
	uint32_t chan_numbers;
	uint32_t source_clock_hz;
	sai_sync_mode_t rx_sync_mode;
	sai_sync_mode_t tx_sync_mode;
	uint8_t fifo_water_mark;
	enum sai_mode working_mode;
};

uint32_t get_sai_id(I2S_Type *base);

void *__sai_base(uint32_t id);

int sai_drv_setup(struct sai_device *sai_dev, struct sai_cfg *sai_config);

void sai_drv_exit(struct sai_device *dev);

int sai_read(struct sai_device *dev, uint8_t *addr, size_t len);

int sai_write(struct sai_device *dev, uint8_t *addr, size_t len);

void sai_fifo_read(struct sai_device *dev, uint8_t *addr, size_t len);

void sai_fifo_write(struct sai_device *dev, uint8_t *addr, size_t len);

void sai_enable_rx(struct sai_device *dev, bool enable_irq);

void sai_enable_tx(struct sai_device *dev, bool enable_irq);

void reset_rx_fifo(struct sai_device *dev);

void reset_tx_fifo(struct sai_device *dev);

void sai_enable_irq(struct sai_device *dev, bool rx_irq, bool tx_irq);

void sai_disable_irq(struct sai_device *dev, bool rx_irq, bool tx_irq);

static inline void __sai_rx_reset(void *base)
{
	SAI_RxEnable(base, false);

	SAI_RxClearStatusFlags(base, I2S_RCSR_FEF_MASK);

	/* Reset FIFO */
	SAI_RxSoftwareReset(base, kSAI_ResetTypeFIFO);
}

static inline void __sai_tx_reset(void *base)
{
	SAI_TxEnable(base, false);

	SAI_TxClearStatusFlags(base, I2S_TCSR_FEF_MASK);

	/* Reset FIFO */
	SAI_TxSoftwareReset(base, kSAI_ResetTypeFIFO);
}

static inline void __sai_enable_irq(void *base, bool rx_irq, bool tx_irq)
{
	if (rx_irq) {
		/* Enable Rx interrupt */
#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
		/* Use FIFO request interrupt and fifo error*/
		SAI_RxEnableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FRIE_MASK);
#else
		SAI_RxEnableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FWIE_MASK);
#endif /* FSL_FEATURE_SAI_FIFO_COUNT */
	}

	if (tx_irq) {
		/* Enable Tx interrupt */
#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
		/* Use FIFO request interrupt and fifo error*/
		SAI_TxEnableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FRIE_MASK);
#else
		SAI_TxEnableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FWIE_MASK);
#endif
	}
}

static inline void __sai_disable_irq(void *base, bool rx_irq, bool tx_irq)
{
	if (rx_irq) {
		/* Disable Rx interrupt */
 #if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
		SAI_RxDisableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FRIE_MASK);
#else
		SAI_RxDisableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FWIE_MASK);
#endif /* FSL_FEATURE_SAI_FIFO_COUNT */
	}

	if (tx_irq) {
		/* Disable Tx interrupt */
#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
		/* Use FIFO request interrupt and fifo error*/
		SAI_TxDisableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FRIE_MASK);
#else
		SAI_TxDisableInterrupts(base,
				I2S_TCSR_FEIE_MASK | I2S_TCSR_FWIE_MASK);
#endif
	}
}

static inline void __sai_enable_rx(void *base, bool enable_irq)
{
	if (enable_irq)
		__sai_enable_irq(base, true, false);

	/* Enable Rx transfer */
	SAI_RxEnable(base, true);
}

static inline void __sai_enable_tx(void *base, bool enable_irq)
{
	if (enable_irq)
		__sai_enable_irq(base, false, true);

	/* Enable Tx transfer */
	SAI_TxEnable(base, true);
}

static inline bool __sai_rx_error(void *base)
{
	return (((((I2S_Type *)base)->RCSR) & (uint32_t)I2S_RCSR_FEF_MASK) != 0UL);
}

static inline bool __sai_tx_error(void *base)
{
	return (((((I2S_Type *)base)->TCSR) & (uint32_t)I2S_TCSR_FEF_MASK) != 0UL);
}

static inline void *__sai_rx_fifo_addr(void *base, unsigned int line)
{
	return (void *)SAI_RxGetDataRegisterAddress(base, line);
}

static inline void *__sai_tx_fifo_addr(void *base, unsigned int line)
{
	return (void *)SAI_TxGetDataRegisterAddress(base, line);
}

#endif /* _SAI_DRV_H_ */
