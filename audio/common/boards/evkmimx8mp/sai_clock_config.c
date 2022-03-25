/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_audiomix.h"

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

void sai_clock_setup()
{
	int active_sai[] = {3, 5};
	int i;

	/* init AUDIO PLL1 run at 393216000HZ */
	CLOCK_InitAudioPll1(&g_audioPll1Config);
	/* init AUDIO PLL2 run at 361267200HZ */
	CLOCK_InitAudioPll2(&g_audioPll2Config);

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

	for (i = 0; i < ARRAY_SIZE(active_sai); i++) {
		/* Set SAI source to AUDIO PLL1 393216000HZ */
		CLOCK_SetRootMux(sai_clock_root[active_sai[i] - 1], kCLOCK_SaiRootmuxAudioPll1);
		/* Set root clock to 393216000HZ / 16 = 24.576MHz */
		CLOCK_SetRootDivider(sai_clock_root[active_sai[i] - 1], 1U, 16U);
	}
}
