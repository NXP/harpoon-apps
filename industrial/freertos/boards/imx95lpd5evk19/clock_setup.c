/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "fsl_clock.h"
#include "app_board.h"
#include "clock_setup.h"

void BOARD_TpmClockSetup(void)
{
	hal_clk_t hal_clk = {
		.clk_id = hal_clock_tpm2,
		.pclk_id = hal_clock_osc24m,
		.div = 1,
		.enable_clk = true,
		.clk_round_opt = hal_clk_round_auto,
	};

	HAL_ClockSetRootClk(&hal_clk);

	hal_clk.clk_id = hal_clock_tpm4;
	HAL_ClockSetRootClk(&hal_clk);
}

void clock_setup_flexcan(void)
{
	hal_clk_t hal_flexcanclk = {
		.clk_id = FLEXCAN_CLOCK_ROOT,
		.pclk_id = hal_clock_osc24m,
		.div = 1,
		.enable_clk = true,
		.clk_round_opt = hal_clk_round_auto,
	};

	HAL_ClockSetRootClk(&hal_flexcanclk);
}

int clock_get_flexcan_clock(uint32_t *rate)
{
	uint64_t flexcan_rate = HAL_ClockGetIpFreq(FLEXCAN_CLOCK_ROOT);

	if (rate && flexcan_rate <= UINT32_MAX) {
		*rate = flexcan_rate;
		return 0;
	} else {
		return -1;
	}
}

void clock_setup_enetc(void)
{
	/* ENET Clocks should already be initialized by Host OS, ensure that they are ON 
	 * and at the right rate
	 */
	hal_clk_t hal_enetclk = {
		.clk_id = hal_clock_enet,
		.enable_clk = true,
	};
	hal_clk_t hal_enetrefclk = {
		.clk_id = hal_clock_enetref,
		.enable_clk = true,
	};
	uint32_t rate;

	HAL_ClockEnableRootClk(&hal_enetclk);
	rate = HAL_ClockGetIpFreq(hal_clock_enet);
	configASSERT(rate == 666666666);

	HAL_ClockEnableRootClk(&hal_enetrefclk);
	rate = HAL_ClockGetIpFreq(hal_clock_enetref);
	configASSERT(rate == 250000000);
}

uint32_t clock_get_mdio_clock(void)
{
	return HAL_ClockGetIpFreq(hal_clock_enet);
}

uint32_t clock_get_netc_timer_clock(void)
{
	return HAL_ClockGetIpFreq(hal_clock_enet);
}

uint32_t clock_get_tpm_clock(void *base)
{
	if (base == TPM1)
		return HAL_ClockGetIpFreq(hal_clock_busaon);
	else if (base == TPM2)
		return HAL_ClockGetIpFreq(hal_clock_tpm2);
	else if (base == TPM3)
		return HAL_ClockGetIpFreq(hal_clock_buswakeup);
	else if (base == TPM4)
		return HAL_ClockGetIpFreq(hal_clock_tpm4);
	else if (base == TPM5)
		return HAL_ClockGetIpFreq(hal_clock_tpm5);
	else if (base == TPM6)
		return HAL_ClockGetIpFreq(hal_clock_tpm6);
	else
		return 0;
}
