/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "fsl_enet_qos.h"
#include "fsl_gpt.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_NET_TX_CACHEABLE
#define BOARD_NET_RX_CACHEABLE

/* PHY configuration */
#define BOARD_NUM_PHY        1

/* PHY 0 is RTL8211FDI phy */
#define BOARD_PHY0_ADDRESS      (0x01U)           /* Phy address of enet port 1. */
#define BOARD_PHY0_OPS          phyrtl8211f_ops  /* PHY operations. */

#define BOARD_PHY0_TX_LATENCY_100M (650) /* FIXME - Needs calibration, comes from RT1170 which has same phy */
#define BOARD_PHY0_RX_LATENCY_100M (650) /* FIXME - Needs calibration, comes from RT1170 which has same phy */
#define BOARD_PHY0_TX_LATENCY_1G   (184)
#define BOARD_PHY0_RX_LATENCY_1G   (569)

/* Network Port configuration */
#define BOARD_NUM_PORTS                1
#define BOARD_NUM_ENET_QOS_PORTS       1

#define BOARD_NUM_MDIO                 1
#define BOARD_PHY0_MDIO_ID             0
#define BOARD_MDIO0_DRV_TYPE           ENET_QOS_t
#define BOARD_MDIO0_DRV_INDEX          (0)

#define BOARD_NET_PORT0_MII_MODE    kENET_QOS_RgmiiMode
#define BOARD_NET_PORT0_DRV_TYPE    ENET_QOS_t
#define BOARD_NET_PORT0_DRV_BASE    ENET_QOS_BASE
#define BOARD_NET_PORT0_DRV_INDEX	(0)
#define BOARD_NET_PORT0_DRV_IRQ0     ENET_QOS_IRQn
#define BOARD_NET_PORT0_DRV_IRQ0_HND ENET_QOS_DriverIRQHandler

#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_0      kENET_QOS_PtpPpsIstance0
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_1      kENET_QOS_PtpPpsIstance2
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_2      kENET_QOS_PtpPpsIstance3
#define BOARD_NET_PORT0_1588_TIMER_EVENT_CHANNEL  kENET_QOS_PtpPpsIstance1

#define BOARD_NUM_GPT           1

#define BOARD_NET_PORT0_1588_TIMER_PPS            kENET_QOS_PtpPpsIstance3 /* FIXME - Need to identify timer channel with PPS output */

#define BOARD_NET_PORT0_PHY_INDEX   (0)

#define BOARD_NUM_GPT                            1

#define BOARD_GPT_0_BASE                         GPT2
#define BOARD_GPT_0_IRQ                          GPT2_IRQn

/* Define device counter instances */
#define BOARD_COUNTER_0_BASE    GPT1
#define BOARD_COUNTER_0_IRQ     GPT1_IRQn

unsigned int BOARD_GPT_clk_src(void *base);
unsigned int BOARD_GPT_clk_freq(void *base);
int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac);

/* Define ENET QOS application dependencies */
#define EXAMPLE_ENET_QOS_BASE   ENET_QOS
#define EXAMPLE_PHY_ADDR        0x01U
#define EXAMPLE_ENET_QOS_OPS    enet_qos_ops
#define EXAMPLE_PHY             phyrtl8211f_ops
#define EXAMPLE_ENET_QOS_IRQ    ENET_QOS_IRQn
#define ENET_PTP_REF_CLK        50000000UL
#define EXAMPLE_ENET_QOS_CLK_FREQ     (CLOCK_GetEnetAxiFreq())

/* Define Flexcan application dependencies */
#define EXAMPLE_CAN                FLEXCAN1
#define EXAMPLE_FLEXCAN_IRQn       CAN_FD1_IRQn
#define EXAMPLE_CAN_CLK_FREQ                                                                    \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootFlexCan1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootFlexCan1)))
/* Set USE_IMPROVED_TIMING_CONFIG macro to use api to calculates the improved CAN / CAN FD timing values. */

#endif /* _APP_BOARD_H_ */
