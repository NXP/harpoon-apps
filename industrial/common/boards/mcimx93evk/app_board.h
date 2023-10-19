/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

/* Define counter application dependencies */
#define BOARD_COUNTER_0_BASE    TPM2
#define BOARD_COUNTER_0_IRQ     TPM2_IRQn

/* Define Flexcan application dependencies */
#define EXAMPLE_CAN             CAN2
#define EXAMPLE_FLEXCAN_IRQn    CAN2_IRQn
#define FLEXCAN_CLOCK_ROOT      (kCLOCK_Root_Can2)
#define FLEXCAN_CLOCK_GATE      kCLOCK_Can2
#define EXAMPLE_CAN_CLK_FREQ    (CLOCK_GetIpFreq(FLEXCAN_CLOCK_ROOT))

#endif /* _APP_BOARD_H_ */
