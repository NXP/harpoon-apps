/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_setup.h"

void board_clock_setup(void)
{
	/* board clock initialization must be run firstly */
	BOARD_TpmClockSetup();
	clock_setup_flexcan();
}

int board_clock_get_flexcan_rate(uint32_t *rate)
{
	return clock_get_flexcan_clock(rate);
}
