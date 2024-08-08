/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "hal_clock_platform.h"

static void BOARD_TpmClockSetup(void)
{
    hal_clk_t hal_clk = {
        .clk_id = hal_clock_tpm2,
        .pclk_id = hal_clock_osc24m,
        .div = 1,
        .enable_clk = true,
        .clk_round_opt = hal_clk_round_auto,
    };

    HAL_ClockSetRootClk(&hal_clk);

    hal_clk.clk_id = hal_clock_tpm4;
    HAL_ClockSetRootClk(&hal_clk);
}

void BOARD_InitClocks(void)
{
    /* board clock initialization must be run firstly */
    BOARD_TpmClockSetup();
}
