/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#if __has_include("app_board.h")
#include "app_board.h"
#endif
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME        "MIMX95-EVK"
#define MANUFACTURER_NAME "NXP"
#define BOARD_DOMAIN_ID   (1U)
/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#define BOARD_DEBUG_UART_BASEADDR LPUART3_BASE
#define BOARD_DEBUG_UART_INSTANCE (3U)
#define BOARD_DEBUG_UART_CLK_FREQ HAL_ClockGetIpFreq(hal_clock_lpuart3)
#define BOARD_UART_IRQ            LPUART3_IRQn
#define BOARD_UART_IRQ_HANDLER    LPUART3_IRQHandler

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitDebugConsole(void);
void BOARD_InitMemory(void);
void BOARD_InitPlatform(void);
void BOARD_RdcInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
