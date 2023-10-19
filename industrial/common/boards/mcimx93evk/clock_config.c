/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "app_board.h"
#include "clock_init.h"

static void BOARD_TpmClockSetup(void)
{
	const clock_root_config_t tpmClkCfg = {
  		.clockOff = false,
		.mux = 0, /* 24MHz oscillator source */
		.div = 1
	};

	CLOCK_SetRootClock(kCLOCK_Root_Tpm2, &tpmClkCfg);
	CLOCK_SetRootClock(kCLOCK_Root_Tpm4, &tpmClkCfg);
}

static void clock_setup_flexcan(void)
{
	/* clang-format off */
	const clock_root_config_t flexcanClkCfg = {
		.clockOff = false,
		.mux = 2, /* SYS_PLL_PFD1_DIV2 source clock */
		.div = 10
	};

	CLOCK_DisableClock(FLEXCAN_CLOCK_GATE);
	CLOCK_SetRootClock(FLEXCAN_CLOCK_ROOT, &flexcanClkCfg);
	CLOCK_EnableClock(FLEXCAN_CLOCK_GATE);
}

void board_clock_setup(void)
{
	/* board clock initialization must be run firstly */
	BOARD_ClockSourceFreqInit();
	BOARD_LpuartClockSetup();
	BOARD_TpmClockSetup();
	clock_setup_flexcan();
}
