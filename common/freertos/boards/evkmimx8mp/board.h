/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_
#include "clock_config.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* TODO Implement CLOCK_GetRootClockFreq() */
#define kCLOCK_Root_Uart4                 4
#define CLOCK_GetRootClockFreq(clockRoot) OSC24M_CLK_FREQ

/*! @brief The board name */
#define BOARD_NAME        "MIMX8MP-EVK"
#define MANUFACTURER_NAME "NXP"
#define BOARD_DOMAIN_ID   (1U)
/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#define BOARD_DEBUG_UART_BASEADDR UART4_BASE
#define BOARD_DEBUG_UART_INSTANCE (4U)
#define BOARD_DEBUG_UART_CLK_FREQ (CLOCK_GetRootClockFreq(kCLOCK_Root_Uart4))
#define BOARD_UART_IRQ         UART4_IRQn
#define BOARD_UART_IRQ_HANDLER UART4_IRQHandler

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitDebugConsole(void);
void BOARD_InitMemory(void);
void BOARD_RdcInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
