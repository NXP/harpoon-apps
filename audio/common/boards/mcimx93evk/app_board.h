/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#ifndef CONFIG_AUD_DISABLE_ENET
#include "genavb_sdk.h"
#endif

#define CODEC_SAI_MASTER_SLAVE		kSAI_Master
#define CODEC_SAI_TX_SYNC_MODE		kSAI_ModeAsync

#define CS42448_I2C_INSTANCE			(4U)
#define CS42448_I2C_CLOCK 				(kCLOCK_Lpi2c4)
#define CS42448_I2C_CLOCK_FREQ			CLOCK_GetIpFreq(kCLOCK_Root_Lpi2c4)
#define CS42448_RESET_GPIO_CLOCK_GATE	kCLOCK_Gpio2
#define CS42448_RESET_GPIO				GPIO2
#define CS42448_RESET_GPIO_PIN			(25U)

#define WM8962_I2C					(LPI2C1)
#define WM8962_I2C_INSTANCE			(1U)
#define WM8962_I2C_CLOCK 			(kCLOCK_Lpi2c1)
#define WM8962_I2C_CLOCK_FREQ		CLOCK_GetIpFreq(kCLOCK_Root_Lpi2c1)

/* Support for only a single audio PLL on the on-board codec for now */
#define WM8962_SUPPORTED_RATES			{48000, 96000}

#define CS42448_SUPPORTED_RATES			{48000U, 96000U, 192000U}

/* SAI3 definitions */
#define SAI3_SAI					(SAI3)
#define SAI3_MASTER_SLAVE			(CODEC_SAI_MASTER_SLAVE)
#define SAI3_CLK_FREQ				CLOCK_GetIpFreq(kCLOCK_Root_Sai3)
#define SAI3_TX_SYNC_MODE			(CODEC_SAI_TX_SYNC_MODE)
#define SAI3_WM8962_RX_SYNC_MODE	(kSAI_ModeSync)
#define SAI3_CS42448_RX_SYNC_MODE	(kSAI_ModeAsync)

#define DEMO_SAI_CHANNEL		(0)

#define DEMO_AUDIO_DATA_CHANNEL	(2U)
#define DEMO_AUDIO_BIT_WIDTH	kSAI_WordWidth32bits

#endif /* _APP_BOARD_H_ */