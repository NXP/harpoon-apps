/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "sai_clock_config.h"

#if (CONFIG_GENAVB_ENABLE == 1)
#include "fsl_enet_qos.h"

static void enet_qos_clock_config(void)
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

static void gpt_clock_config(void)
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
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

void BOARD_InitClocks(void)
{
#if (CONFIG_GENAVB_ENABLE == 1)
	enet_qos_clock_config();

	gpt_clock_config();
#endif
	sai_clock_setup();
}
