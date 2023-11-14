/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#ifndef CONFIG_AUD_DISABLE_ENET
#include "fsl_clock.h"
#include "fsl_enet_qos.h"
#include "fsl_tpm.h"
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

/* Network Port configuration */
#ifndef CONFIG_AUD_DISABLE_ENET

#define BOARD_NET_TX_CACHEABLE
#define BOARD_NET_RX_CACHEABLE

/* PHY configuration */
#define BOARD_NUM_PHY			1

/* PHY 0 is RTL8211FDI phy */
#define BOARD_PHY0_ADDRESS				(0x01U)			/* Phy address of enet port 1. */
#define BOARD_PHY0_OPS					phyrtl8211f_ops	/* PHY operations. */

#define BOARD_PHY0_TX_LATENCY_100M		(650)			/* FIXME - Needs calibration, comes from RT1170 which has same phy */
#define BOARD_PHY0_RX_LATENCY_100M		(650)			/* FIXME - Needs calibration, comes from RT1170 which has same phy */
#define BOARD_PHY0_TX_LATENCY_1G		(184)
#define BOARD_PHY0_RX_LATENCY_1G		(569)

#define BOARD_NUM_PORTS					1
#define BOARD_NUM_ENET_QOS_PORTS		1

#define BOARD_NUM_MDIO					1
#define BOARD_PHY0_MDIO_ID				0
#define BOARD_MDIO0_DRV_TYPE			ENET_QOS_t
#define BOARD_MDIO0_DRV_INDEX			(0)

#define BOARD_NET_PORT0_MII_MODE		kENET_QOS_RgmiiMode
#define BOARD_NET_PORT0_DRV_TYPE		ENET_QOS_t
#define BOARD_NET_PORT0_DRV_BASE		ENET_QOS_BASE
#define BOARD_NET_PORT0_DRV_INDEX		(0)
#define BOARD_NET_PORT0_DRV_IRQ0		ENET_QOS_IRQn
#define BOARD_NET_PORT0_DRV_IRQ0_HND	ENET_QOS_DriverIRQHandler

#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_0		kENET_QOS_PtpPpsIstance0
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_1		kENET_QOS_PtpPpsIstance2
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_2		kENET_QOS_PtpPpsIstance3
#define BOARD_NET_PORT0_1588_TIMER_EVENT_CHANNEL	kENET_QOS_PtpPpsIstance1

#define BOARD_NET_PORT0_1588_TIMER_PPS				kENET_QOS_PtpPpsIstance3 /* FIXME - Need to identify timer channel with PPS output */

#define BOARD_NET_PORT0_PHY_INDEX					(0)

#define BOARD_NUM_TPM								1

#define BOARD_TPM_0_BASE							TPM2
#define BOARD_TPM_0_IRQ								TPM2_IRQn

#define BOARD_GENAVB_TIMER_0_IRQ					BOARD_TPM_0_IRQ
#define BOARD_GENAVB_TIMER_0_IRQ_HANDLER			BOARD_TPM_0_IRQ_HANDLER

unsigned int BOARD_TPM_clk_src(void *base);
unsigned int BOARD_TPM_clk_freq(void *base);
int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac);

#endif /* CONFIG_AUD_DISABLE_ENET */

#endif /* _APP_BOARD_H_ */