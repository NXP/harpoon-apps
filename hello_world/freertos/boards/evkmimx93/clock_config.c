/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "board.h"

void BOARD_InitClocks(void)
{
    /* board clock initialization must be run firstly */
    BOARD_ClockSourceFreqInit();
    BOARD_LpuartClockSetup();
}
