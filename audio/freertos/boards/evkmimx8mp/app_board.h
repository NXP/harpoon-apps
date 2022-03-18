/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

/* Definitions for WM8960 */
#define WM8960_SAI			(I2S3)

#define WM8960_SAI_MASTER_SLAVE		kSAI_Master

#define WM8960_SAI_CLK_FREQ				\
    (CLOCK_GetPllFreq(kCLOCK_AudioPll1Ctrl)		\
     / (CLOCK_GetRootPreDivider(kCLOCK_RootSai3))	\
     / (CLOCK_GetRootPostDivider(kCLOCK_RootSai3)))


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


#if (USE_CODEC_HIFIBERRY == 1)
#define DEMO_SAI			(HIFIBERRY_SAI)
#define DEMO_CODEC_ID			(CODEC_ID_HIFIBERRY)
#define DEMO_SAI_MASTER_SLAVE		(HIFIBERRY_SAI_MASTER_SLAVE)
#define DEMO_AUDIO_MASTER_CLOCK		(HIFIBERRY_SAI_CLK_FREQ)
#elif (USE_CODEC_WM8960 == 1)
#define DEMO_SAI			(WM8960_SAI)
#define DEMO_CODEC_ID			(CODEC_ID_WM8524)
#define DEMO_SAI_MASTER_SLAVE		(WM8960_SAI_MASTER_SLAVE)
#define DEMO_AUDIO_MASTER_CLOCK		(WM8960_SAI_CLK_FREQ)
#else
#error "No default codec defined (flag use USE_CODEC_xxx)"
#endif

#define DEMO_SAI_CHANNEL		(0)

/*set Bclk source to Mclk clock*/
#define DEMO_SAI_CLOCK_SOURCE		(1U)

#define DEMO_SAI_TX_SYNC_MODE		kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE		kSAI_ModeSync
#define DEMO_SAI_MCLK_OUTPUT		true

#define DEMO_AUDIO_DATA_CHANNEL		(2U)
#define DEMO_AUDIO_BIT_WIDTH		kSAI_WordWidth32bits
#define DEMO_AUDIO_SAMPLE_RATE		(kSAI_SampleRate44100Hz)

#define PLAT_WITH_AUDIOMIX

#endif /* _APP_BOARD_H_ */
