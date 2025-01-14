/*
 * Copyright 2023, 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Define device counter instances */
#define BOARD_COUNTER_0_BASE    TPM2
#define BOARD_COUNTER_0_IRQ     TPM2_IRQn
#define BOARD_COUNTER_0_IRQ_PRIO OS_IRQ_PRIO_DEFAULT
#define BOARD_COUNTER_1_BASE    TPM4
#define BOARD_COUNTER_1_IRQ     TPM4_IRQn
#define BOARD_COUNTER_1_IRQ_PRIO (OS_IRQ_PRIO_DEFAULT + 1)

#endif /* _APP_BOARD_H_ */
