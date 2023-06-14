/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

static void clock_config_enet(void)
{
	/* enetClk 250MHz */
	const clock_root_config_t enetClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
		.div = 4
	};

	/* enetRefClk 250MHz (For 125MHz TX_CLK ) */
	const clock_root_config_t enetRefClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_ENETREF_ClockRoot_MuxSysPll1Pfd0Div2, // 500MHz
		.div = 2
	};

	CLOCK_SetRootClock(kCLOCK_Root_WakeupAxi, &enetClkCfg);
	CLOCK_SetRootClock(kCLOCK_Root_EnetRef, &enetRefClkCfg);
}

void board_clock_setup(void)
{
	BOARD_ClockSourceFreqInit();

	BOARD_LpuartClockSetup();
	clock_config_enet();
}
