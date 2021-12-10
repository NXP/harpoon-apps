/*
 * Copyright 2021 NXP
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

struct sai_device {
	void *sai_handle;
	void *sai_rx_handle;
	void *sai_tx_handle;
	void *sai_base;
};

typedef void (*sai_callback_t)(uint8_t status, void *user_data);

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
	enum sai_mode working_mode;
};

uint32_t get_sai_id(I2S_Type *base);

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

#endif /* _SAI_DRV_H_ */
