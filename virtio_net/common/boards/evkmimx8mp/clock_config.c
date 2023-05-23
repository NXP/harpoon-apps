/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_enet.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

static void clock_config_enet(void)
{
	CLOCK_DisableClock(kCLOCK_Enet1);

	CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);
	CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3); /* SYSTEM PLL1 divided by 3: 266Mhz, aligned with Linux kernel*/

	CLOCK_SetRootDivider(kCLOCK_RootEnetTimer, 1U, 1U);
	CLOCK_SetRootMux(kCLOCK_RootEnetTimer, kCLOCK_EnetTimerRootmuxSysPll2Div10); /* SYSTEM PLL2 divided by 10: 100Mhz */

	CLOCK_EnableClock(kCLOCK_Enet1);
}

void board_clock_setup(void)
{
	clock_config_enet();
}
