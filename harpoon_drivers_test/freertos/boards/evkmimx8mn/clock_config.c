/*
 * Copyright 2022 NXP
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

void board_clock_setup(void)
{
    enet_test_clock_setup();
}
