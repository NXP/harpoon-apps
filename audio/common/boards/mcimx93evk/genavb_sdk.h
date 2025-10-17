/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _GENAVB_SDK_H_
#define _GENAVB_SDK_H_

#include <stdarg.h>
#include <stdint.h>

#include "fsl_clock.h"
#include "fsl_enet_qos.h"
#include "fsl_tpm.h"

/* Network Port configuration */
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

#define BOARD_NET_PORT0_1588_TIMER_EVENT_CHANNEL	kENET_QOS_PtpPpsIstance1

#define BOARD_ENET_QOS0_1588_TIMER_CHANNEL_0		kENET_QOS_PtpPpsIstance0
#define BOARD_ENET_QOS0_1588_TIMER_CHANNEL_1		kENET_QOS_PtpPpsIstance2
#define BOARD_ENET_QOS0_1588_TIMER_CHANNEL_2		kENET_QOS_PtpPpsIstance3

#define BOARD_ENET_QOS0_1588_TIMER_PPS				kENET_QOS_PtpPpsIstance3 /* FIXME - Need to identify timer channel with PPS output */

#define BOARD_NET_PORT0_PHY_INDEX					(0)

#define BOARD_NUM_TPM								1

#define BOARD_TPM_0_BASE							TPM2
#define BOARD_TPM_0_IRQ								TPM2_IRQn
#define BOARD_TPM_REC_BASE							TPM2
#define BOARD_TPM_REC_SW_SAMPLING						1 /* Enable MCR with SW sampling */
#define BOARD_TPM_REC_BASE_FREQ							49152000U

#define BOARD_GENAVB_TIMER_0_IRQ					BOARD_TPM_0_IRQ
#define BOARD_GENAVB_TIMER_0_IRQ_HANDLER			BOARD_TPM_0_IRQ_HANDLER

unsigned int BOARD_TPM_clk_src(void *base);
unsigned int BOARD_TPM_clk_src_div(void *base);
unsigned int BOARD_TPM_clk_freq(void *base);
int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac);

uint32_t dev_get_pll_ref_freq(void);
void dev_write_audio_pll_num(uint32_t num);
uint32_t dev_read_audio_pll_num(void);
uint32_t dev_read_audio_pll_denom(void);
uint32_t dev_read_audio_pll_post_div(void);
uint32_t dev_get_enet_core_freq(void *base);
uint32_t dev_get_enet_1588_freq(void *base);
uint32_t dev_get_tpm_counter_freq(void *base);
uint32_t dev_get_tpm_counter_clk_src_div(void *base);

#endif /* _GENAVB_SDK_H_ */
