/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ethernet_setup.h"
#include "hal_power.h"

void netcmix_init(void)
{
	hal_pwr_s_t pwrst = {
		.did = HAL_POWER_PLATFORM_MIX_SLICE_IDX_NETC,
		.st = hal_power_state_on,
	};

	/* Check if NETCMIX is ON */
	while (HAL_PowerGetState(&pwrst))
	{
	}
}
