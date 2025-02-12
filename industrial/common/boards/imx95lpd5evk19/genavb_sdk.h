/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _GENAVB_SDK_H_
#define _GENAVB_SDK_H_

#include "fsl_netc.h"
#include "fsl_netc_soc.h"
#include "fsl_netc_timer.h"
#include "fsl_tpm.h"

/*! @brief NET configuration */
#define BOARD_NUM_PORTS          1

#define BOARD_NUM_ENETC_PORTS    1
#define BOARD_NUM_NETC_SWITCHES  0
#define BOARD_NUM_NETC_PORTS     0

#define BOARD_NUM_MSGINTR        1
#define BOARD_MSGINTR0_BASE MSGINTR2

/* ENETC0 */
#define BOARD_NET_PORT0_DRV_TYPE   ENETC_1G_t
#define BOARD_NET_PORT0_DRV_INDEX  (0)
#define BOARD_NET_PORT0_DRV_BASE   kNETC_ENETC0PSI0
#define BOARD_NET_PORT0_PHY_INDEX  (0)
#define BOARD_NET_PORT0_MII_MODE   kNETC_RgmiiMode
#define BOARD_NET_PORT0_HW_CLOCK   0
#define BOARD_NET_PORT0_DRV_IRQ0     MSGINTR2_IRQn
#define BOARD_NET_PORT0_DRV_IRQ0_HND MSGINTR2_IRQHandler

#define BOARD_NUM_NETC_HW_CLOCK       1

#define BOARD_NETC_HW_CLOCK_0_ID      0
#define BOARD_NETC_HW_CLOCK_0_OWNER   1
#define BOARD_NETC_HW_CLOCK_0_MSGINTR_BASE MSGINTR2
#define BOARD_NETC_HW_CLOCK_0_MSGINTR_CH 0
#define BOARD_NETC_HW_CLOCK_0_SELECT kNETC_TimerSystemClk
/* NETC System Clock (ENET) is divided by two at NETC Timer */
#define BOARD_NETC_HW_CLOCK_0_FREQ (HAL_ClockGetIpFreq(hal_clock_enet)/2)
#define BOARD_HW_CLOCK0_NUM_TIMERS    2
#define BOARD_HW_CLOCK0_TIMER0_ID     1 /* Alarm2 */
#define BOARD_HW_CLOCK0_TIMER0_ENABLED
#define BOARD_HW_CLOCK0_TIMER0_IRQ_ENABLED
#define BOARD_HW_CLOCK0_TIMER1_ID     2 /* Fiper1 */
#define BOARD_HW_CLOCK0_TIMER1_ENABLED
#define BOARD_HW_CLOCK0_TIMER1_PPS

#define BOARD_NUM_NETC_EMDIO 1
#define BOARD_NUM_NETC_PORT_EMDIO 1
#define BOARD_NETC_PORT_EMDIO_PORT0 kNETC_ENETC0EthPort
#define BOARD_NETC_MDIO_FREQ (HAL_ClockGetIpFreq(hal_clock_enet))

#define BOARD_NET_RX_CACHEABLE 1
#define BOARD_NET_TX_CACHEABLE 1

#define BOARD_NUM_MDIO                     1
#define BOARD_MDIO0_DRV_TYPE               NETC_PORT_EMDIO_t
#define BOARD_MDIO0_DRV_INDEX              0

#define BOARD_NUM_PHY        1

#define BOARD_PHY0_MDIO_ID            0
#define BOARD_PHY0_ADDRESS            0x1U
#define BOARD_PHY0_OPS                phyrtl8211f_ops
#define BOARD_PHY0_RX_LATENCY_100M    733
#define BOARD_PHY0_TX_LATENCY_100M    1297
#define BOARD_PHY0_RX_LATENCY_1G      530
#define BOARD_PHY0_TX_LATENCY_1G      202

#define BOARD_NUM_TPM                            1

#define BOARD_TPM_0_BASE                         TPM4
#define BOARD_TPM_0_IRQ                          TPM4_IRQn

#define BOARD_GENAVB_TIMER_0_IRQ                 BOARD_TPM_0_IRQ
#define BOARD_GENAVB_TIMER_0_IRQ_HANDLER         BOARD_TPM_0_IRQ_HANDLER

unsigned int BOARD_TPM_clk_src(void *base);
unsigned int BOARD_TPM_clk_freq(void *base);
int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac);

uint32_t dev_get_tpm_counter_freq(void *base);

#endif /* _GENAVB_SDK_H_ */
