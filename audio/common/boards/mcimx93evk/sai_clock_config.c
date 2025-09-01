/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "codec_config.h"
#include "rtos_abstraction_layer.h"
#include "sai_drv.h"
#include "sai_config.h"

void sai_clock_setup(void)
{
	int i;

	if (sai_active_list_nelems == 0)
		rtos_assert(false, "No SAI enabled!");

	/* Enable SAI clocks */
	for (i = 0; i < sai_active_list_nelems; i++) {
		const clock_root_config_t saiClkCfg = {
			.clockOff = false,
			.mux = 1, // select audiopll1out source(393216000 Hz)
			.div = sai_active_list[i].audio_pll_div,
		};
		
		CLOCK_SetRootClock(sai_active_list[i].root_clk_id, &saiClkCfg);
		CLOCK_EnableClock(sai_active_list[i].clk_id);
	}
}

uint32_t sai_select_audio_pll_mux(int sai_id, int srate)
{
	return kCLOCK_AudioPll1Out;
}

uint32_t get_sai_clock_freq(unsigned int sai_active_index)
{
	rtos_assert(sai_active_index < sai_active_list_nelems, "%u not a valid active sai_active_index", sai_active_index);

	return CLOCK_GetIpFreq(sai_active_list[sai_active_index].root_clk_id);
}
