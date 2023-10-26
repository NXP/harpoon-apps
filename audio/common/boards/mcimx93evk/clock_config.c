/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sai_clock_config.h"
#include "clock_init.h"

void BOARD_InitClocks(void)
{
	BOARD_ClockSourceFreqInit();
	BOARD_LpuartClockSetup();
	sai_clock_setup();
}
