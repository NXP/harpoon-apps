/*
 * Copyright 2022, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

static void clock_setup_enet_qos(void)
{
	CLOCK_DisableClock(kCLOCK_Enet_Qos);

	CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3);
	CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);

	CLOCK_SetRootMux(kCLOCK_RootEnetQos, kCLOCK_EnetQosRootmuxSysPll2Div8);
	CLOCK_SetRootDivider(kCLOCK_RootEnetQos, 1U, 1U);

	CLOCK_SetRootMux(kCLOCK_RootEnetQosTimer, kCLOCK_EnetQosTimerRootmuxSysPll2Div10);
	CLOCK_SetRootDivider(kCLOCK_RootEnetQosTimer, 1U, 1U);

	CLOCK_EnableClock(kCLOCK_Enet_Qos);
}

static void clock_setup_gpt(void)
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

static void clock_setup_flexcan(void)
{
	CLOCK_DisableClock(kCLOCK_Can1);

	/* Set FLEXCAN1 source to SYSTEM PLL1 800MHZ */
	CLOCK_SetRootMux(kCLOCK_RootFlexCan1, kCLOCK_FlexCanRootmuxSysPll1);
	/* Set root clock to 800MHZ / 10 = 80MHZ */
	CLOCK_SetRootDivider(kCLOCK_RootFlexCan1, 2U, 5U);

	CLOCK_EnableClock(kCLOCK_Can1);
}

void board_clock_setup(void)
{
	clock_setup_enet_qos();
	clock_setup_flexcan();
	clock_setup_gpt();
}

int board_clock_get_flexcan_rate(uint32_t *rate)
{
	if (rate) {
		*rate = (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootFlexCan1)) / (CLOCK_GetRootPostDivider(kCLOCK_RootFlexCan1)));
		return 0;
	} else {
		return -1;
	}

}
