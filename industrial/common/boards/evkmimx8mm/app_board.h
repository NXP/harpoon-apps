/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "fsl_enet.h"
#include "fsl_gpt.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_NET_TX_CACHEABLE
#define BOARD_NET_RX_CACHEABLE

/* Network Port configuration */
#define BOARD_NUM_PORTS        1
#define BOARD_NUM_ENET_PORTS   1

/* PHY configuration */
#define BOARD_NUM_PHY        1

/* PHY 0 is RTL8211FDI phy */
#define BOARD_PHY0_ADDRESS      (0x00U)           /* Phy address of enet port 0. */
#define BOARD_PHY0_OPS          phyar8031_ops    /* PHY operations. */

#define BOARD_PHY0_TX_LATENCY_100M (810)          /* FIXME - Needs calibration */
#define BOARD_PHY0_RX_LATENCY_100M (810)          /* FIXME - Needs calibration */

#define BOARD_NUM_MDIO             1
#define BOARD_PHY0_MDIO_ID         0
#define BOARD_MDIO0_DRV_TYPE       ENET_t
#define BOARD_MDIO0_DRV_INDEX      (0)

#define BOARD_NET_PORT0_MII_MODE    kENET_RgmiiMode
#define BOARD_NET_PORT0_DRV_TYPE    ENET_t
#define BOARD_NET_PORT0_DRV_BASE    ENET_BASE
#define BOARD_NET_PORT0_DRV_INDEX	(0)
#define BOARD_NET_PORT0_DRV_IRQ0     ENET_IRQn
#define BOARD_NET_PORT0_DRV_IRQ0_HND ENET_DriverIRQHandler
#define BOARD_NET_PORT0_DRV_IRQ1     ENET_1588_IRQn
#define BOARD_NET_PORT0_DRV_IRQ1_HND ENET_1588_Timer_DriverIRQHandler


#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_0      kENET_PtpTimerChannel1
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_1      kENET_PtpTimerChannel3
#define BOARD_NET_PORT0_1588_TIMER_CHANNEL_2      kENET_PtpTimerChannel4
#define BOARD_NET_PORT0_1588_TIMER_EVENT_CHANNEL  kENET_PtpTimerChannel2

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

#endif /* _APP_BOARD_H_ */
