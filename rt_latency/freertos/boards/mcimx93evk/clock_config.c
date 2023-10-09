/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "clock_init.h"

static void BOARD_TpmClockSetup(void)
{
    const clock_root_config_t tpmClkCfg = {
        .clockOff = false,
        .mux = 0, /* 24MHz oscillator source */
        .div = 1
    };

    CLOCK_SetRootClock(kCLOCK_Root_Tpm2, &tpmClkCfg);
    CLOCK_SetRootClock(kCLOCK_Root_Tpm4, &tpmClkCfg);
}

void BOARD_InitClocks(void)
{
    /* board clock initialization must be run firstly */
    BOARD_ClockSourceFreqInit();
    BOARD_LpuartClockSetup();
    BOARD_TpmClockSetup();
}
