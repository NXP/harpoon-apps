/*
 * Copyright 2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#include "genavb_sdk.h"

extern const fracn_pll_init_t g_audioPllCfg;

#define PLL_PARENT_CLOCK_XTAL	24000000U
#define PLL_NUMERATOR_MFN_GET(val) (((val) & PLL_NUMERATOR_MFN_MASK) >> PLL_NUMERATOR_MFN_SHIFT)

/*
 * 24MHz XTAL oscillator as primary clock source for all PLLs
 * The imx_pll API from GenAVB/TSN expects pll_ref to match: pll_out = ref x (div + num/denum) [1]
 * While the i.MX 93 has: pll_out = (24MHz / rdiv) * (mfi + (mfn / mfd)) / odiv [2]
 * So, align the returned pll_ref to take into account the pre and post dividers
 */
uint32_t dev_get_pll_ref_freq(void)
{
	uint32_t rdiv   = g_audioPllCfg.rdiv;
	uint32_t odiv  = g_audioPllCfg.odiv;

	return PLL_PARENT_CLOCK_XTAL / (rdiv * odiv);
}

/*
 * Audio Pll tuning
 */
void dev_write_audio_pll_num(uint32_t num)
{
	/* FIXME add sanity checks on the numerator: -2 <= mfn/mfd <= 2 and and make sure
	 * num does not overflow the 30-bits signed mfn field.
	 */
	AUDIOPLL->NUMERATOR.RW = PLL_NUMERATOR_MFN(num);
}

uint32_t dev_read_audio_pll_num(void)
{
	return PLL_NUMERATOR_MFN_GET(AUDIOPLL->NUMERATOR.RW);
}

uint32_t dev_read_audio_pll_denom(void)
{
	return g_audioPllCfg.mfd;
}

uint32_t dev_read_audio_pll_post_div(void)
{
	return g_audioPllCfg.mfi;
}

/*
 * Enet QOS module frequency (clk_csr_i)
 */
uint32_t dev_get_enet_core_freq(void *base)
{
	if (base == ENET_QOS)
		return CLOCK_GetIpFreq(kCLOCK_Root_WakeupAxi);
	else
		return 0; //TODO
}

/*
 * Enet Qos 1588 timer frequency (ptp_ref_i)
 */
uint32_t dev_get_enet_1588_freq(void *base)
{
	if (base == ENET_QOS)
		return CLOCK_GetIpFreq(kCLOCK_Root_EnetTimer2);
	else
		return 0;
}

/*
 * TPM counter input frequency (lptpm_clk)
 */
uint32_t dev_get_tpm_counter_freq(void *base)
{
	if (base == TPM1)
		return CLOCK_GetIpFreq(kCLOCK_Root_BusAon);
	else if (base == TPM2)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm2);
	else if (base == TPM3)
		return CLOCK_GetIpFreq(kCLOCK_Root_BusWakeup);
	else if (base == TPM4)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm4);
	else if (base == TPM5)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm5);
	else if (base == TPM6)
		return CLOCK_GetIpFreq(kCLOCK_Root_Tpm6);
	else
		return 0;
}

uint32_t dev_get_tpm_counter_clk_src_div(void *base)
{
	if (base == TPM1)
		return CLOCK_GetRootClockDiv(kCLOCK_Root_BusAon);
	else if (base == TPM2)
		return CLOCK_GetRootClockDiv(kCLOCK_Root_Tpm2);
	else if (base == TPM3)
		return CLOCK_GetRootClockDiv(kCLOCK_Root_BusWakeup);
	else if (base == TPM4)
		return CLOCK_GetRootClockDiv(kCLOCK_Root_Tpm4);
	else if (base == TPM5)
		return CLOCK_GetRootClockDiv(kCLOCK_Root_Tpm5);
	else if (base == TPM6)
		return CLOCK_GetRootClockDiv(kCLOCK_Root_Tpm6);
	else
		return 1;
}
