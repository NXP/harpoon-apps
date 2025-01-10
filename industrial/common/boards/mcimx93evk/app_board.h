/*
 * Copyright 2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "genavb_sdk.h"

/* Define ENET QOS application dependencies */
#define ENET_PTP_REF_CLK                CLOCK_GetIpFreq(kCLOCK_Root_EnetTimer2)
#define EXAMPLE_ENET_QOS_BASE           ENET_QOS
#define EXAMPLE_PHY_ADDR                (0x01U)
#define EXAMPLE_PHY                     BOARD_PHY0_OPS
#define EXAMPLE_ENET_QOS_IRQ            ENET_QOS_IRQn
#define EXAMPLE_ENET_QOS_CLK_FREQ       (CLOCK_GetIpFreq(kCLOCK_Root_WakeupAxi))

/* Define device counter instances */
#define BOARD_COUNTER_0_BASE    TPM2
#define BOARD_COUNTER_0_IRQ     TPM2_IRQn
#define BOARD_COUNTER_0_IRQ_PRIO OS_IRQ_PRIO_DEFAULT

/* Define Flexcan application dependencies */
#define EXAMPLE_CAN             CAN2
#define EXAMPLE_FLEXCAN_IRQn    CAN2_IRQn
#define FLEXCAN_CLOCK_ROOT      (kCLOCK_Root_Can2)
#define FLEXCAN_CLOCK_GATE      kCLOCK_Can2

#endif /* _APP_BOARD_H_ */
