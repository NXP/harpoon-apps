/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "clock_init.h"
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * AUDIOPLL1/AUDIOPLL1OUT
 *
 * VCO = (24MHz / rdiv) * (mfi + (mfn / mfd))  = 3,932,160,000 Hz
 * Output = VCO / odiv = 393.216 MHz
 */
const fracn_pll_init_t g_audioPllCfg = {
	.rdiv = 1,
	.mfi = 163,
	.mfn = 1680000,
	.mfd = 2000000,
	.odiv = 10
};

static inline void BOARD_AudioClockSetup(void)
{
	/* ROM has already initialized PLL */
	CLOCK_PllInit(AUDIOPLL, &g_audioPllCfg);
}

void BOARD_InitClocks(void)
{
	BOARD_ClockSourceFreqInit();
	BOARD_AudioClockSetup();
	BOARD_LpuartClockSetup();
}
