/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexcan.h"

void board_clock_setup(void)
{
    /* Set FLEXCAN1 source to SYSTEM PLL1 800MHZ */
    CLOCK_SetRootMux(kCLOCK_RootFlexCan1, kCLOCK_FlexCanRootmuxSysPll1);
    /* Set root clock to 800MHZ / 10 = 80MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootFlexCan1, 2U, 5U);
}
