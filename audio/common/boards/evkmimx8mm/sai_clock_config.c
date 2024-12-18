/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

#include "codec_config.h"
#include "rtos_abstraction_layer.h"
#include "sai_drv.h"
#include "sai_config.h"

static const uintptr_t sai_clock_root[] = {kCLOCK_RootSai1, kCLOCK_RootSai2,
	kCLOCK_RootSai3, kCLOCK_RootSai4, kCLOCK_RootSai5, kCLOCK_RootSai6, 0};

static const uintptr_t sai_clock[] = {kCLOCK_Sai1, kCLOCK_Sai2,
	kCLOCK_Sai3, kCLOCK_Sai4, kCLOCK_Sai5, kCLOCK_Sai6, 0};

void sai_clock_setup(void)
{
	int i;

	if (sai_active_list_nelems == 0)
		rtos_assert(false, "No SAI enabled!");

	/* Enable SAI clocks */
	for (i = 0; i < sai_active_list_nelems; i++) {
		int sai_id;
		uint32_t root_mux_apll;

		sai_id = get_sai_id(sai_active_list[i].sai_base);
		rtos_assert(sai_id, "SAI%d enabled but not supported in this platform!", i);

		/* Set SAI source to AUDIO PLL 393216000HZ */
		switch (sai_active_list[i].audio_pll) {
			case kCLOCK_AudioPll1Ctrl:
				root_mux_apll = kCLOCK_SaiRootmuxAudioPll1;
				break;
			case kCLOCK_AudioPll2Ctrl:
				root_mux_apll = kCLOCK_SaiRootmuxAudioPll2;
				break;
			default:
				rtos_assert(false, "Invalid Audio PLL! (%d)", sai_active_list[i].audio_pll);
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

uint32_t sai_select_audio_pll_mux(int sai_id, int srate)
{
	uint32_t root_mux_apll;

	root_mux_apll = __get_pll_rootmux_from_srate(srate);
	CLOCK_SetRootMux(sai_clock_root[sai_id - 1], root_mux_apll);

	return __get_pll_from_srate(srate);
}

static uint32_t get_sai_clock_root(uint32_t id)
{
	return sai_clock_root[id];
}

uint32_t get_sai_clock_freq(unsigned int sai_active_index)
{
	uint32_t sai_clock_root;
	int sai_id;

	rtos_assert(sai_active_index < sai_active_list_nelems, "%u not a valid active index", sai_active_index);

	sai_id = get_sai_id(sai_active_list[sai_active_index].sai_base);
	rtos_assert(sai_id, "SAI%d enabled but not supported in this platform!", sai_active_index);

	sai_clock_root = get_sai_clock_root(sai_id - 1);

	return CLOCK_GetPllFreq(sai_active_list[sai_active_index].audio_pll) / CLOCK_GetRootPreDivider(sai_clock_root) / CLOCK_GetRootPostDivider(sai_clock_root);
}
