/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "os/assert.h"
#include "codec_config.h"
#include "sai_drv.h"
#include "sai_config.h"

static const uintptr_t sai_clock_root[] = {kCLOCK_Root_Sai1, kCLOCK_Root_Sai2,
	kCLOCK_Root_Sai3};

static const uintptr_t sai_clock_gate[] = {kCLOCK_Sai1, kCLOCK_Sai2,
	kCLOCK_Sai3};

void sai_clock_setup(void)
{
	int i;

	if (sai_active_list_nelems == 0)
		os_assert(false, "No SAI enabled!");

	/* Enable SAI clocks */
	for (i = 0; i < sai_active_list_nelems; i++) {
		int sai_id;

		sai_id = get_sai_id(sai_active_list[i].sai_base);
		os_assert(sai_id, "SAI%d enabled but not supported in this platform!", i);

		const clock_root_config_t saiClkCfg = {
			.clockOff = false,
			.mux = 1, // select audiopll1out source(393216000 Hz)
			.div = sai_active_list[i].audio_pll_div,
		};
		
		CLOCK_SetRootClock(sai_clock_root[sai_id - 1], &saiClkCfg);
		CLOCK_EnableClock(sai_clock_gate[sai_id - 1]);
	}
}

uint32_t sai_select_audio_pll_mux(int sai_id, int srate)
{
	return kCLOCK_AudioPll1Out;
}

static uint32_t get_sai_clock_root(uint32_t id)
{
	return sai_clock_root[id];
}

uint32_t get_sai_clock_freq(unsigned int sai_active_index)
{
	uint32_t sai_clock_root;
	int sai_id;

	os_assert(sai_active_index < sai_active_list_nelems, "%u not a valid active index", sai_active_index);

	sai_id = get_sai_id(sai_active_list[sai_active_index].sai_base);
	os_assert(sai_id, "SAI%d enabled but not supported in this platform!", sai_active_index);

	sai_clock_root = get_sai_clock_root(sai_id - 1);

	return CLOCK_GetIpFreq(sai_clock_root);
}
