/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_init.h"

void BOARD_ClockSourceFreqInit(void)
{
	g_clockSourceFreq[kCLOCK_AudioPll1Out]    = 393216000U;
	g_clockSourceFreq[kCLOCK_AudioPll1]       = 393216000U;
}

void BOARD_LpuartClockSetup(void)
{
	const clock_root_config_t uartClkCfg = {
		.clockOff = false,
		.mux = 0, /* 24MHz oscillator source */
		.div = 1
	};

	CLOCK_SetRootClock(kCLOCK_Root_Lpuart2, &uartClkCfg);
}
