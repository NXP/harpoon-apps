/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexcan.h"

static void i2c_test_clock_setup(void)
{
    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
    CLOCK_EnableClock(kCLOCK_I2c3);
}

void board_clock_setup(void)
{
    /* Set FLEXCAN1 source to SYSTEM PLL1 800MHZ */
    CLOCK_SetRootMux(kCLOCK_RootFlexCan1, kCLOCK_FlexCanRootmuxSysPll1);
    /* Set root clock to 800MHZ / 10 = 80MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootFlexCan1, 2U, 5U);
    i2c_test_clock_setup();
}
