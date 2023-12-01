/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#ifndef CONFIG_AUD_DISABLE_ENET
#include "genavb_sdk.h"
#endif

#define WM8962_SAI_MASTER_SLAVE		kSAI_Master

#define WM8962_SAI_CLK_FREQ			CLOCK_GetIpFreq(kCLOCK_Root_Sai3)
#define WM8962_SAI_TX_SYNC_MODE		kSAI_ModeAsync
#define WM8962_SAI_RX_SYNC_MODE		kSAI_ModeSync

#define BOARD_CODEC_I2C				(LPI2C1)
#define BOARD_CODEC_I2C_INSTANCE	(1U)
#define BOARD_CODEC_I2C_CLOCK_FREQ	CLOCK_GetIpFreq(kCLOCK_Root_Lpi2c1)

/* SAI3 definitions */
#define SAI3_SAI				(SAI3)
#define SAI3_MASTER_SLAVE		(WM8962_SAI_MASTER_SLAVE)
#define SAI3_CLK_FREQ			(WM8962_SAI_CLK_FREQ)
#define SAI3_TX_SYNC_MODE		(WM8962_SAI_TX_SYNC_MODE)
#define SAI3_RX_SYNC_MODE		(WM8962_SAI_RX_SYNC_MODE)

#define DEMO_SAI_CHANNEL		(0)

#define DEMO_AUDIO_DATA_CHANNEL	(2U)
#define DEMO_AUDIO_BIT_WIDTH	kSAI_WordWidth32bits

/* Support for only a single audio PLL and on-board codec for now */
#define SUPPORTED_RATES			{48000, 96000}

#endif /* _APP_BOARD_H_ */