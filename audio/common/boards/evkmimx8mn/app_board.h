/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#ifndef CONFIG_AUD_DISABLE_ENET
#include "genavb_sdk.h"
#endif

/* Definitions for WM8524 */
#define WM8524_SAI			(I2S3)

#define WM8524_SAI_MASTER_SLAVE		kSAI_Master

#define WM8524_SAI_CLK_FREQ				\
    (CLOCK_GetPllFreq(kCLOCK_AudioPll1Ctrl)		\
     / (CLOCK_GetRootPreDivider(kCLOCK_RootSai3))	\
     / (CLOCK_GetRootPostDivider(kCLOCK_RootSai3)))

#define WM8524_SAI_TX_SYNC_MODE		kSAI_ModeAsync
#define WM8524_SAI_RX_SYNC_MODE		kSAI_ModeSync


/* Definitions for Hifiberry */
#define HIFIBERRY_SAI			(I2S5)

#define BOARD_CODEC_I2C			(I2C3)
#define BOARD_CODEC_I2C_INSTANCE	(3U)
#define BOARD_CODEC_I2C_CLOCK_FREQ	(16000000U)

#define PCM512X_I2C_ADDR		(0x4D)
#define PCM512X_GPIO_LED		(4U)
#define PCM512X_GPIO_OSC44		(6U)
#define PCM512X_GPIO_OSC48		(3U)

#define PCM186X_I2C_ADDR		(0x4A)
#define PCM186X_GPIO_LED		(2U)

#define HIFIBERRY_SAI_MASTER_SLAVE	kSAI_Slave

#define HIFIBERRY_SAI_CLK_FREQ				\
    (CLOCK_GetPllFreq(kCLOCK_AudioPll1Ctrl)		\
     / (CLOCK_GetRootPreDivider(kCLOCK_RootSai5))	\
     / (CLOCK_GetRootPostDivider(kCLOCK_RootSai5)))

#define HIFIBERRY_SAI_TX_SYNC_MODE	kSAI_ModeAsync
#define HIFIBERRY_SAI_RX_SYNC_MODE	kSAI_ModeSync


/* SAI5 definitions */
#define SAI5_SAI			(I2S5)
#define SAI5_MASTER_SLAVE		(HIFIBERRY_SAI_MASTER_SLAVE)
#define SAI5_CLK_FREQ			(HIFIBERRY_SAI_CLK_FREQ)
#define SAI5_TX_SYNC_MODE		(HIFIBERRY_SAI_TX_SYNC_MODE)
#define SAI5_RX_SYNC_MODE		(HIFIBERRY_SAI_RX_SYNC_MODE)

/* SAI3 definitions */
#define SAI3_SAI			(I2S3)
#define SAI3_MASTER_SLAVE		(WM8524_SAI_MASTER_SLAVE)
#define SAI3_CLK_FREQ			(WM8524_SAI_CLK_FREQ)
#define SAI3_TX_SYNC_MODE		(WM8524_SAI_TX_SYNC_MODE)
#define SAI3_RX_SYNC_MODE		(WM8524_SAI_RX_SYNC_MODE)

#define DEMO_SAI_CHANNEL		(0)

#define DEMO_AUDIO_DATA_CHANNEL		(2U)
#define DEMO_AUDIO_BIT_WIDTH		kSAI_WordWidth32bits

#define SUPPORTED_RATES				{44100, 48000, 88200, 96000, 176400, 192000}

#endif /* _APP_BOARD_H_ */
