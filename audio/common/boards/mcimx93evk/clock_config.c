/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "clock_init.h"

void BOARD_InitClocks(void)
{
	BOARD_ClockSourceFreqInit();
	BOARD_LpuartClockSetup();
}
