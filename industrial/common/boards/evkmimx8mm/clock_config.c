/*
 * Copyright 2022 NXP
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
	CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3);

	CLOCK_SetRootMux(kCLOCK_RootEnetTimer, kCLOCK_EnetTimerRootmuxSysPll2Div10);
	CLOCK_SetRootDivider(kCLOCK_RootEnetTimer, 1U, 1U);

	CLOCK_EnableClock(kCLOCK_Enet1);
}

static void clock_config_gpt(void)
{
	CLOCK_DisableClock(kCLOCK_Gpt1);
	CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxOsc24M);
	CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 1U);
	CLOCK_EnableClock(kCLOCK_Gpt1);

	CLOCK_DisableClock(kCLOCK_Gpt2);
	CLOCK_SetRootMux(kCLOCK_RootGpt2, kCLOCK_GptRootmuxOsc24M);
	CLOCK_SetRootDivider(kCLOCK_RootGpt2, 1U, 1U);
	CLOCK_EnableClock(kCLOCK_Gpt2);
}

void board_clock_setup(void)
{
	clock_config_enet();
	clock_config_gpt();
}
