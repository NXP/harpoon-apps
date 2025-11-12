/*
 * Copyright 2022, 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "fsl_common.h"

#include "rtos_abstraction_layer.h"

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

static inline void BOARD_AudioClockSetup(void)
{
	/* Init Audio PLLs */
	CLOCK_InitAudioPll1(&g_audioPll1Config);
	CLOCK_InitAudioPll2(&g_audioPll2Config);

	CLOCK_EnableRoot(kCLOCK_RootAudioAhb);
}

void BOARD_InitClocks(void)
{
	BOARD_AudioClockSetup();
}

/* Avoid precision error due to fractional part in CLOCK_GetPllFreq()
 * and return the right frequency configured above
 */
uint32_t BOARD_GetAudioPLLFreq(int pll_id)
{
	uint32_t pll_freq = 0;

	switch (pll_id) {
	case kCLOCK_AudioPll1Ctrl:
		pll_freq = 393216000;
		break;
	case kCLOCK_AudioPll2Ctrl:
		pll_freq = 361267200;
		break;
	default:
		rtos_assert(false, "Invalid Audio PLL! (%d)", pll_id);
		break;
	}

	return pll_freq;
}
