/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_clock.h"

static void lpi2c_clock_setup(void)
{
    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
        .mux = 0, /* 24MHz oscillator source */
        .div = 1
    };

    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &lpi2cClkCfg);
    CLOCK_EnableClock(kCLOCK_Lpi2c1);
}

void board_clock_setup(void)
{
    BOARD_ClockSourceFreqInit();
    BOARD_LpuartClockSetup();
    lpi2c_clock_setup();
}
