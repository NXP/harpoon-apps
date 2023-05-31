/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_enet.h"

static void enet_test_clock_setup(void)
{
    CLOCK_DisableClock(kCLOCK_Enet1);

    CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3);

    CLOCK_EnableClock(kCLOCK_Enet1);
}

static void i2c_test_clock_setup(void)
{
    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
    CLOCK_EnableClock(kCLOCK_I2c3);
}

void board_clock_setup(void)
{
    enet_test_clock_setup();
    i2c_test_clock_setup();
}
