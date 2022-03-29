/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

#include "os/assert.h"
#include "codec_config.h"
#include "sai_drv.h"
#include "sai_config.h"

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
	int i;

	if (sai_active_list_nelems == 0)
		os_assert(false, "No SAI enabled!");

	/* Init Audio PLLs */
	for (i = 0; i < sai_active_list_nelems; i++) {
		bool apll1_enabled = false, apll2_enabled = false;

		switch (sai_active_list[i].audio_pll) {
			case kCLOCK_AudioPll1Ctrl:
				/* init AUDIO PLL1 run at 393216000HZ */
				if (apll1_enabled == false) {
					CLOCK_InitAudioPll1(&g_audioPll1Config);
					apll1_enabled = true;
				}
				break;
			case kCLOCK_AudioPll2Ctrl:
				/* init AUDIO PLL2 run at 361267200HZ */
				if (apll2_enabled == false) {
					CLOCK_InitAudioPll2(&g_audioPll2Config);
					apll2_enabled = true;
				}
				break;
			default:
				os_assert(false, "Invalid Audio PLL! (%d)", sai_active_list[i].audio_pll);
				break;
		}
	}

	CLOCK_EnableRoot(kCLOCK_RootAudioAhb);

	/* Enable SAI clocks */
	for (i = 0; i < sai_active_list_nelems; i++) {
		int sai_id;
		uint32_t root_mux_apll;

		sai_id = get_sai_id(sai_active_list[i].sai_base);
		os_assert(sai_id, "SAI%d enabled but not supported in this platform!", i);

		/* Set SAI source to AUDIO PLL 393216000HZ */
		switch (sai_active_list[i].audio_pll) {
			case kCLOCK_AudioPll1Ctrl:
				root_mux_apll = kCLOCK_SaiRootmuxAudioPll1;
				break;
			case kCLOCK_AudioPll2Ctrl:
				root_mux_apll = kCLOCK_SaiRootmuxAudioPll2;
				break;
			default:
				os_assert(false, "Invalid Audio PLL! (%d)", sai_active_list[i].audio_pll);
				break;
		}
		CLOCK_SetRootMux(sai_clock_root[sai_id - 1], root_mux_apll);

		/* Set root clock to 393216000HZ / 16 = 24.576MHz */
		CLOCK_SetRootDivider(sai_clock_root[sai_id - 1],
				sai_active_list[i].audio_pll_mul,
				sai_active_list[i].audio_pll_div);
		CLOCK_EnableClock(sai_clock[sai_id - 1]);
	}
}

uint32_t get_sai_clock_root(uint32_t id)
{
	return sai_clock_root[id];
}
