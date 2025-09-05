/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_clock.h"
#include "rtos_abstraction_layer.h"

#include "rtos_apps/audio/audio_app.h"

void audio_app_sai_clock_setup(void)
{
	int i;

	if (audio_app_sai_active_list_nelems == 0)
		rtos_assert(false, "No SAI enabled!");

	/* Enable SAI clocks */
	for (i = 0; i < audio_app_sai_active_list_nelems; i++) {
		rtos_assert(audio_app_sai_active_list[i].audio_pll_div < UINT8_MAX, "pll_div out of bound");
		const clock_root_config_t saiClkCfg = {
			.clockOff = false,
			.mux = 1, // select audiopll1out source(393216000 Hz)
			.div = audio_app_sai_active_list[i].audio_pll_div,
		};
		
		CLOCK_SetRootClock(audio_app_sai_active_list[i].root_clk_id, &saiClkCfg);
		CLOCK_EnableClock(audio_app_sai_active_list[i].clk_id);
	}
}

uint32_t audio_app_sai_select_audio_pll_mux(unsigned int index, uint32_t srate)
{
	return kCLOCK_AudioPll1Out;
}

uint32_t audio_app_sai_get_clock_freq(unsigned int index)
{
	rtos_assert(index < audio_app_sai_active_list_nelems, "%u not a valid active index", index);

	return CLOCK_GetIpFreq(audio_app_sai_active_list[index].root_clk_id);
}
