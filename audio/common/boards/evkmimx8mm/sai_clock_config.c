/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Fractional PLLs: Fout = ((mainDiv+dsm/65536) * refSel) / (preDiv * 2^ postDiv) */
/* AUDIO PLL1 configuration */
const ccm_analog_frac_pll_config_t g_audioPll1Config = {
	.refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
	.mainDiv = 262U,
	.dsm     = 9437U,
	.preDiv  = 2U,
	.postDiv = 3U, /*!< AUDIO PLL1 frequency  = 393216000HZ */
};

/* AUDIO PLL2 configuration */
const ccm_analog_frac_pll_config_t g_audioPll2Config = {
	.refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
	.mainDiv = 361U,
	.dsm     = 17511U,
	.preDiv  = 3U,
	.postDiv = 3U, /*!< AUDIO PLL2 frequency  = 361267200HZ */
};

static const uintptr_t sai_clock_root[] = {kCLOCK_RootSai1, kCLOCK_RootSai2,
	kCLOCK_RootSai3, kCLOCK_RootSai4, kCLOCK_RootSai5, kCLOCK_RootSai6, 0};

static const uintptr_t sai_clock[] = {kCLOCK_Sai1, kCLOCK_Sai2,
	kCLOCK_Sai3, kCLOCK_Sai4, kCLOCK_Sai5, kCLOCK_Sai6, 0};

void sai_clock_setup(void)
{
	int active_sai[] = {3, 5};
	int i;

	/* init AUDIO PLL1 run at 393216000HZ */
	CLOCK_InitAudioPll1(&g_audioPll1Config);
	/* init AUDIO PLL2 run at 361267200HZ */
	CLOCK_InitAudioPll2(&g_audioPll2Config);

	CLOCK_EnableRoot(kCLOCK_RootAudioAhb);

	for (i = 0; i < ARRAY_SIZE(active_sai); i++) {
		/* Set SAI source to AUDIO PLL1 393216000HZ */
		CLOCK_SetRootMux(sai_clock_root[active_sai[i] - 1], kCLOCK_SaiRootmuxAudioPll1);
		/* Set root clock to 393216000HZ / 16 = 24.576MHz */
		CLOCK_SetRootDivider(sai_clock_root[active_sai[i] - 1], 1U, 16U);
		CLOCK_EnableClock(sai_clock[active_sai[i] - 1]);
	}
}
