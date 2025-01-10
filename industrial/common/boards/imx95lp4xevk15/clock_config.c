/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "app_board.h"

static void BOARD_TpmClockSetup(void)
{
	hal_clk_t hal_clk = {
		.clk_id = hal_clock_tpm2,
		.pclk_id = hal_clock_osc24m,
		.div = 1,
		.enable_clk = true,
		.clk_round_opt = hal_clk_round_auto,
	};

	HAL_ClockSetRootClk(&hal_clk);
}

static void clock_setup_flexcan(void)
{
	/* clang-format off */
	hal_clk_t hal_flexcanclk = {
		.clk_id = FLEXCAN_CLOCK_ROOT,
		.pclk_id = hal_clock_osc24m,
		.div = 1,
		.enable_clk = true,
		.clk_round_opt = hal_clk_round_auto,
	};

	HAL_ClockSetRootClk(&hal_flexcanclk);
}

void board_clock_setup(void)
{
	/* board clock initialization must be run firstly */
	BOARD_TpmClockSetup();
	clock_setup_flexcan();
}

int board_clock_get_flexcan_rate(uint32_t *rate)
{
	uint64_t flexcan_rate = HAL_ClockGetIpFreq(FLEXCAN_CLOCK_ROOT);

	if (rate && flexcan_rate <= UINT32_MAX) {
		*rate = flexcan_rate;
		return 0;
	} else
		return -1;
}
