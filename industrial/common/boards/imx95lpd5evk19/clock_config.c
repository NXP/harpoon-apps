/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_setup.h"

void board_clock_setup(void)
{
	/* board clock initialization must be run firstly */
	BOARD_TpmClockSetup();
	clock_setup_flexcan();
#if defined(CONFIG_USE_GENAVB) && (CONFIG_USE_GENAVB == 1)
	clock_setup_enetc();
#endif
}

int board_clock_get_flexcan_rate(uint32_t *rate)
{
	return clock_get_flexcan_clock(rate);
}
