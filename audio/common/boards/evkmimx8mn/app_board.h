/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#ifndef CONFIG_AUD_DISABLE_ENET
#include "fsl_clock.h"
#include "fsl_enet.h"
#include "fsl_enet_mdio.h"
#include "fsl_gpt.h"
#include "fsl_phyar8031.h"
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
#define SAI5_CODEC_ID                   (CODEC_ID_HIFIBERRY)

/* SAI3 definitions */
#define SAI3_SAI			(I2S3)
#define SAI3_MASTER_SLAVE		(WM8524_SAI_MASTER_SLAVE)
#define SAI3_CLK_FREQ			(WM8524_SAI_CLK_FREQ)
#define SAI3_TX_SYNC_MODE		(WM8524_SAI_TX_SYNC_MODE)
#define SAI3_RX_SYNC_MODE		(WM8524_SAI_RX_SYNC_MODE)
#define SAI3_CODEC_ID                   (CODEC_ID_WM8524)

#define DEMO_SAI_CHANNEL		(0)

#define DEMO_AUDIO_DATA_CHANNEL		(2U)
#define DEMO_AUDIO_BIT_WIDTH		kSAI_WordWidth32bits

/* Network Port configuration */
#ifndef CONFIG_AUD_DISABLE_ENET
#define BOARD_NUM_PORTS        1
#define BOARD_NUM_ENET_PORTS   1

/* PHY configuration */
#define BOARD_NUM_PHY        1

/* PHY 0 is RTL8211FDI phy */
#define BOARD_PHY0_ADDRESS      (0x00U)           /* Phy address of enet port 0. */
#define BOARD_PHY0_MDIO_BASE    ENET_BASE         /* MDIO MAC controller base address. */
#define BOARD_PHY0_MDIO_OPS     &enet_ops         /* MDIO MAC controller operations. */
#define BOARD_PHY0_OPS          &phyar8031_ops    /* PHY operations. */

#define BOARD_PHY0_TX_LATENCY_100M (810)          /* FIXME - Needs calibration */
#define BOARD_PHY0_RX_LATENCY_100M (810)          /* FIXME - Needs calibration */

#define BOARD_NET_PORT0_MII_MODE                 kENET_RgmiiMode
#define BOARD_NET_PORT0_DRV_TYPE                 ENET_t
#define BOARD_NET_PORT0_DRV_BASE                 ENET_BASE
#define BOARD_NET_PORT0_DRV_INDEX                (0)
#define BOARD_NET_PORT0_DRV_IRQ0                 ENET_IRQn
#define BOARD_NET_PORT0_DRV_IRQ0_HND             ENET_DriverIRQHandler
#define BOARD_NET_PORT0_DRV_IRQ1                 ENET_1588_IRQn
#define BOARD_NET_PORT0_DRV_IRQ1_HND             ENET_1588_Timer_DriverIRQHandler

#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_0     kENET_PtpTimerChannel1
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_1     kENET_PtpTimerChannel3
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_2     kENET_PtpTimerChannel4
#define BOARD_NET_PORT0_1588_TIMER_EVENT_CHANNEL kENET_PtpTimerChannel2

#define BOARD_NET_PORT0_PHY_INDEX                (0)

#define BOARD_NET_TX_CACHEABLE
#define BOARD_NET_RX_CACHEABLE

#define BOARD_GPT_0_BASE                         GPT2
#define BOARD_GPT_0_IRQ                          GPT2_IRQn

#define BOARD_GPT_1_BASE                         GPT1
#define BOARD_GPT_1_IRQ                          GPT1_IRQn

#define BOARD_GPT_1_CHANNEL                      kGPT_InputCapture_Channel2
#define BOARD_GPT_1_INTERRUPT_MASK               kGPT_InputCapture2InterruptEnable
#define BOARD_GPT_1_STATUS_FLAG_MASK             kGPT_InputCapture2Flag

#define BOARD_GPT_1_CLK_EXT_FREQ                 24000000U
#define BOARD_GPT_1_CLK_SOURCE_TYPE              kGPT_ClockSource_Periph

unsigned int BOARD_GPT_clk_src(void *base);
unsigned int BOARD_GPT_clk_freq(void *base);
int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac);

#endif /* CONFIG_AUD_DISABLE_ENET */

#endif /* _APP_BOARD_H_ */
