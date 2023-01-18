/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"

static void board_clock_init(void)
{
    /* LPUART2 clock root selections */
    g_clockSourceFreq[kCLOCK_Osc24M]          = 24000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd0Div2] = 500000000U;
    g_clockSourceFreq[kCLOCK_SysPll1Pfd1Div2] = 400000000U;
    g_clockSourceFreq[kCLOCK_VideoPll1Out]    = 2400000000U;
}

static void lpuart_clock_setup(void)
{
    const clock_root_config_t uartClkCfg = {
        .clockOff = false,
        .mux = 0, /* 24MHz oscillator source */
        .div = 1
    };

    CLOCK_SetRootClock(kCLOCK_Root_Lpuart2, &uartClkCfg);
}

void BOARD_InitClocks(void)
{
    /* board clock initialization must be run firstly */
    board_clock_init();
    lpuart_clock_setup();
}
