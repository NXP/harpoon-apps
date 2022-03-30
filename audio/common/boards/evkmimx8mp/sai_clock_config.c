/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_audiomix.h"

#include "app_board.h"
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

/* AUDIOMIX SAI PLL configuration */
const ccm_analog_frac_pll_config_t g_saiPLLConfig = {
	.refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
	.mainDiv = 361U,
	.dsm     = 17511U,
	.preDiv  = 3U,
	.postDiv = 3U, /*!< SAI PLL frequency  = 361267200HZ */
};

static const uintptr_t sai_clock_root[] = {kCLOCK_RootSai1, kCLOCK_RootSai2,
	kCLOCK_RootSai3, 0, kCLOCK_RootSai5, kCLOCK_RootSai6, kCLOCK_RootSai7};

static const uintptr_t audiomix_sai_mclk1[] = {
	kAUDIOMIX_Attach_SAI1_MCLK1_To_SAI1_ROOT,
	kAUDIOMIX_Attach_SAI2_MCLK1_To_SAI2_ROOT,
	kAUDIOMIX_Attach_SAI3_MCLK1_To_SAI3_ROOT,
	0,
	kAUDIOMIX_Attach_SAI5_MCLK1_To_SAI5_ROOT,
	kAUDIOMIX_Attach_SAI6_MCLK1_To_SAI6_ROOT,
	kAUDIOMIX_Attach_SAI7_MCLK1_To_SAI7_ROOT,
};

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
	/* Enable Audio clock to power on the audiomix domain*/
	CLOCK_EnableClock(kCLOCK_Audio);

	/* Power up the audiomix domain by A53 core.*/
	/* Map the audiomix domain to A53 */
	GPC->PGC_CPU_A53_MAPPING |=
		1U << GPC_PGC_CPU_A53_MAPPING_AUDIOMIX_DOMAIN_SHIFT;
	/* Software request to trigger power up the domain */
	GPC->PU_PGC_SW_PUP_REQ |=
		1U << GPC_PU_PGC_SW_PUP_REQ_AUDIOMIX_SW_PUP_REQ_SHIFT;


	/* Waiting the GPC_PU_PGC_SW_PUP_REQ_AUDIOMIX_SW_PUP_REQ bit self-cleared after power up */
	while(GPC->PU_PGC_SW_PUP_REQ &
			(1U << GPC_PU_PGC_SW_PUP_REQ_AUDIOMIX_SW_PUP_REQ_SHIFT));

	/* Do the handshake to make sure the NOC bus ready after power up the AUDIOMIX domain. */
	GPC->PU_PWRHSK |= 1U << GPC_PU_PWRHSK_GPC_AUDIOMIX_NOC_PWRDNREQN_SHIFT;
	while(!(GPC->PU_PWRHSK &
			(1U << GPC_PU_PWRHSK_GPC_AUDIOMIX_PWRDNACKN_SHIFT)));

	/* init SAI PLL run at 361267200HZ */
	AUDIOMIX_InitAudioPll(AUDIOMIX, &g_saiPLLConfig);

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

		/* SAI bit clock source */
		AUDIOMIX_AttachClk(AUDIOMIX, audiomix_sai_mclk1[sai_id - 1]);
	}
}

uint32_t get_sai_clock_root(uint32_t id)
{
	return sai_clock_root[id];
}
