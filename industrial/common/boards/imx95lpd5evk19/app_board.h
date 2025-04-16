/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "genavb_sdk.h"

/* Define device counter instances */
#define BOARD_COUNTER_0_BASE    TPM2
#define BOARD_COUNTER_0_IRQ     TPM2_IRQn
#define BOARD_COUNTER_0_IRQ_PRIO OS_IRQ_PRIO_DEFAULT

/* Define Flexcan application dependencies */
#define EXAMPLE_CAN             CAN2
#define EXAMPLE_FLEXCAN_IRQn    CAN2_IRQn
#define FLEXCAN_CLOCK_ROOT      (hal_clock_can2)
#define EXAMPLE_CAN_CLK_FREQ    (HAL_ClockGetIpFreq(FLEXCAN_CLOCK_ROOT))

#endif /* _APP_BOARD_H_ */
