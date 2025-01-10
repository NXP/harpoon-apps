/*
 * Copyright 2022-2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "genavb_sdk.h"

/* Define device counter instances */
#define BOARD_COUNTER_0_BASE    GPT1
#define BOARD_COUNTER_0_IRQ     GPT1_IRQn
#define BOARD_COUNTER_0_IRQ_PRIO OS_IRQ_PRIO_DEFAULT

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
/* Set USE_IMPROVED_TIMING_CONFIG macro to use api to calculates the improved CAN / CAN FD timing values. */

#endif /* _APP_BOARD_H_ */
