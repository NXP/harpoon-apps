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
    const clock_ip_name_t enet_clock[] = ENET_CLOCKS;
    uint32_t instance = ENET_GetInstance(ENET);

    CLOCK_DisableClock(enet_clock[instance]);

    CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3);

    CLOCK_EnableClock(enet_clock[instance]);
}

void board_clock_setup(void)
{
	clock_config_enet();
}
