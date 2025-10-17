/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "fsl_common.h"

#include "clock_config.h"

#include "rtos_abstraction_layer.h"

#include "rtos_apps/audio/audio_app.h"

void audio_app_sai_clock_setup(void)
{
	int i;

	if (audio_app_sai_active_list_nelems == 0)
		rtos_assert(false, "No SAI enabled!");

	/* Enable SAI clocks */
	for (i = 0; i < audio_app_sai_active_list_nelems; i++) {
		uint32_t root_mux_apll;

		/* Set SAI source to AUDIO PLL1 (393216000 HZ) or AUDIO PLL2 (361267200 HZ) */
		switch (audio_app_sai_active_list[i].audio_pll) {
			case kCLOCK_AudioPll1Ctrl:
				root_mux_apll = kCLOCK_SaiRootmuxAudioPll1;
				break;
			case kCLOCK_AudioPll2Ctrl:
				root_mux_apll = kCLOCK_SaiRootmuxAudioPll2;
				break;
			default:
				rtos_assert(false, "Invalid Audio PLL! (%d)", audio_app_sai_active_list[i].audio_pll);
				break;
		}

		CLOCK_SetRootMux(audio_app_sai_active_list[i].root_clk_id, root_mux_apll);

		/* Set SAI root clock to PLL Freq / 16 = (24.576MHz or 22.5792MHz) */
		CLOCK_SetRootDivider(audio_app_sai_active_list[i].root_clk_id,
				audio_app_sai_active_list[i].audio_pll_mul,
				audio_app_sai_active_list[i].audio_pll_div);
		CLOCK_EnableClock(audio_app_sai_active_list[i].clk_id);
	}
}

uint32_t audio_app_sai_get_clock_freq(unsigned int index)
{
	clock_root_control_t sai_clock_root;

	rtos_assert(index < audio_app_sai_active_list_nelems, "%u not a valid active index", index);

	sai_clock_root = audio_app_sai_active_list[index].root_clk_id;

	return BOARD_GetAudioPLLFreq(audio_app_sai_active_list[index].audio_pll) / CLOCK_GetRootPreDivider(sai_clock_root) / CLOCK_GetRootPostDivider(sai_clock_root);
}
