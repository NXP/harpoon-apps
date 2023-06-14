/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "mmu.h"
#include "uart.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ClockSourceFreqInit(void)
{
    g_clockSourceFreq[kCLOCK_Osc24M]          = 24000000U;
    g_clockSourceFreq[kCLOCK_SysPll1]         = 4000000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd0]     = 1000000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd0Div2] = 500000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd1]     = 800000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd1Div2] = 400000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd2]     = 625000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd2Div2] = 312500000U;
    g_clockSourceFreq[kCLOCK_AudioPll1Out]    = 393216000U;
    g_clockSourceFreq[kCLOCK_AudioPll1]       = 393216000U;
}

void BOARD_LpuartClockSetup(void)
{
    const clock_root_config_t uartClkCfg = {
        .clockOff = false,
        .mux = 0, /* 24MHz oscillator source */
        .div = 1
    };

    CLOCK_SetRootClock(kCLOCK_Root_Lpuart2, &uartClkCfg);
}

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uart_init();
}

/* Initialize MMU, configure memory attributes for each region */
void BOARD_InitMemory(void)
{
    MMU_init();
}

void BOARD_RdcInit(void)
{
}
