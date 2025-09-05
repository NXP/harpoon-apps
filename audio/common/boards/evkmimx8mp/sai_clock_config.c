/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "fsl_common.h"
#include "fsl_audiomix.h"

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

		/* Set SAI source to AUDIO PLL 393216000HZ */
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

		/* Set root clock to 393216000HZ / 16 = 24.576MHz */
		CLOCK_SetRootDivider(audio_app_sai_active_list[i].root_clk_id,
				audio_app_sai_active_list[i].audio_pll_mul,
				audio_app_sai_active_list[i].audio_pll_div);

		/* SAI bit clock source */
		AUDIOMIX_AttachClk(AUDIOMIX, audio_app_sai_active_list[i].clk_id);
	}
}

static uint32_t __get_pll_rootmux_from_srate(uint32_t srate)
{
	uint32_t root_mux_apll;

	if (srate % 44100 == 0) {
		/* Audio PLL for frequencies multiple of 44100 Hz */
		root_mux_apll = kCLOCK_SaiRootmuxAudioPll2;
	} else {
		/* Audio PLL for frequencies multiple of 48000 Hz */
		root_mux_apll = kCLOCK_SaiRootmuxAudioPll1;
	}

	return root_mux_apll;
}

static uint32_t __get_pll_from_srate(uint32_t srate)
{
	uint32_t apll;

	if (srate % 44100 == 0) {
		/* Audio PLL for frequencies multiple of 44100 Hz */
		apll = kCLOCK_AudioPll2Ctrl;
	} else {
		/* Audio PLL for frequencies multiple of 48000 Hz */
		apll = kCLOCK_AudioPll1Ctrl;
	}

	return apll;
}

uint32_t audio_app_sai_select_audio_pll_mux(unsigned int index, uint32_t srate)
{
	uint32_t root_mux_apll;

	root_mux_apll = __get_pll_rootmux_from_srate(srate);
	CLOCK_SetRootMux(audio_app_sai_active_list[index].root_clk_id, root_mux_apll);

	return __get_pll_from_srate(srate);
}

uint32_t audio_app_sai_get_clock_freq(unsigned int index)
{
	clock_root_control_t sai_clock_root;

	rtos_assert(index < audio_app_sai_active_list_nelems, "%u not a valid active index", index);

	sai_clock_root = audio_app_sai_active_list[index].root_clk_id;

	return CLOCK_GetPllFreq(audio_app_sai_active_list[index].audio_pll) / CLOCK_GetRootPreDivider(sai_clock_root) / CLOCK_GetRootPostDivider(sai_clock_root);
}
