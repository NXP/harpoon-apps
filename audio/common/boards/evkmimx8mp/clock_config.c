/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "fsl_audiomix.h"

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

/* AUDIOMIX SAI PLL configuration */
const ccm_analog_frac_pll_config_t g_saiPLLConfig = {
	.refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
	.mainDiv = 361U,
	.dsm     = 17511U,
	.preDiv  = 3U,
	.postDiv = 3U, /*!< SAI PLL frequency  = 361267200HZ */
};

static inline void BOARD_AudioClockSetup(void)
{
	/* Init Audio PLLs */
	CLOCK_InitAudioPll1(&g_audioPll1Config);
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

void BOARD_InitClocks(void)
{
	BOARD_AudioClockSetup();
}
