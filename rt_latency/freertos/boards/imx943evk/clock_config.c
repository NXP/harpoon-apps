/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"

static void BOARD_TpmClockSetup(void)
{
    /* clang-format off */
    clk_t clk = {
        .pclkId = kCLOCK_Osc24m,
        .clkId = kCLOCK_Tpm2,
        .clkRoundOpt = SCMI_CLOCK_ROUND_AUTO,
        .rate = 24000000UL,
    };
    /* clang-format on */

    CLOCK_SetParent(&clk);
    CLOCK_SetRate(&clk);
    CLOCK_EnableClock(clk.clkId);

    clk.clkId = kCLOCK_Tpm4;
    CLOCK_SetParent(&clk);
    CLOCK_SetRate(&clk);
    CLOCK_EnableClock(clk.clkId);
}

void BOARD_InitClocks(void)
{
    BOARD_TpmClockSetup();
}
