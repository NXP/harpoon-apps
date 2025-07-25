/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#include "genavb_sdk.h"

#define FracPLL_FDIV_CTL1_DENOM 65536ULL
#define FracPLL_FDIV_CTL1_Offset (8U)
extern const ccm_analog_frac_pll_config_t g_audioPll1Config;


/*
 * 24MHz XTAL oscillator as primary clock source for all PLLs
 * The imx_pll API from GenAVB/TSN expects pll_ref to match: pll_out = ref x (div + num/denum) [1]
 * While the i.MX 8MPlus has: pll_out = (parent_rate * (m + k/65536)) / (p * 2^s) [2]
 * So, align the returned pll_ref to take into account the pre and post dividers
 */
uint32_t dev_get_pll_ref_freq(void)
{
	uint8_t preDiv   = g_audioPll1Config.preDiv;
	uint8_t postDiv  = g_audioPll1Config.postDiv;
	uint32_t parent_clk = CLOCK_GetPllRefClkFreq(kCLOCK_AudioPll1Ctrl);

	return parent_clk / ((uint32_t)preDiv * (1ULL << postDiv));
}

/*
 * Audio Pll tuning
 */
void dev_write_audio_pll_num(uint32_t num)
{
	CCM_ANALOG_TUPLE_REG_OFF(CCM_ANALOG, kCLOCK_AudioPll1Ctrl, FracPLL_FDIV_CTL1_Offset) = (int16_t)num;
}

uint32_t dev_read_audio_pll_num(void)
{
	uint32_t fracCfg2 = CCM_ANALOG_TUPLE_REG_OFF(CCM_ANALOG, kCLOCK_AudioPll1Ctrl, FracPLL_FDIV_CTL1_Offset);
	return (uint32_t)CCM_BIT_FIELD_EXTRACTION(fracCfg2, CCM_ANALOG_AUDIO_PLL1_FDIV_CTL1_PLL_DSM_MASK,
														CCM_ANALOG_AUDIO_PLL1_FDIV_CTL1_PLL_DSM_SHIFT);
}

uint32_t dev_read_audio_pll_denom(void)
{
	return FracPLL_FDIV_CTL1_DENOM;
}

uint32_t dev_read_audio_pll_post_div(void)
{
	return (uint32_t)g_audioPll1Config.mainDiv;
}

/*
 * Enet QOS module frequency (ipg_clk)
 */
uint32_t dev_get_enet_core_freq(void *base)
{
	if (base == ENET_QOS)
		return CLOCK_GetPllFreq(kCLOCK_SystemPll2Ctrl) / 8 /
			CLOCK_GetRootPreDivider(kCLOCK_RootEnetQos) /
			CLOCK_GetRootPostDivider(kCLOCK_RootEnetQos);
	else
		return 0;
}

/*
 * Enet QOS module update clocks (RGMII TX_CLK) on link speed update.
 */
void BOARD_enet_qos_clock_fix(void *base, enet_qos_mii_speed_t miiSpeed)
{

	if (base != ENET_QOS)
		return;

	switch (miiSpeed) {
	case kENET_QOS_MiiSpeed1000M:
		/* Generate 125M root clock for 1000Mbps. */
		CLOCK_SetRootDivider(kCLOCK_RootEnetQos, 1U, 1U);
		CLOCK_SetRootMux(kCLOCK_RootEnetQos,
		kCLOCK_EnetQosRootmuxSysPll2Div8); /* SYSTEM PLL2 divided by 8: 125Mhz */
		break;
	case kENET_QOS_MiiSpeed100M:
		/* Generate 25M root clock for 100Mbps. */
		CLOCK_SetRootDivider(kCLOCK_RootEnetQos, 1U, 2U);
		CLOCK_SetRootMux(kCLOCK_RootEnetQos,
		kCLOCK_EnetQosRootmuxSysPll2Div20); /* SYSTEM PLL2 divided by 20: 50Mhz */
		break;
	case kENET_QOS_MiiSpeed10M:
		/* Generate 2.5M root clock for 10Mbps. */
		CLOCK_SetRootDivider(kCLOCK_RootEnetQos, 4U, 5U);
		CLOCK_SetRootMux(kCLOCK_RootEnetQos,
		kCLOCK_EnetQosRootmuxSysPll2Div20); /* SYSTEM PLL2 divided by 20: 50Mhz */
		break;
	default:
		/* Generate 125M root clock. */
		CLOCK_SetRootDivider(kCLOCK_RootEnetQos, 1U, 1U);
		CLOCK_SetRootMux(kCLOCK_RootEnetQos,
		kCLOCK_EnetQosRootmuxSysPll2Div8); /* SYSTEM PLL2 divided by 8: 125Mhz */
		break;
	}
}

/*
 * Enet 1588 timer frequency (ipg_clk_time)
 */
uint32_t dev_get_enet_1588_freq(void *base)
{
	if (base == ENET_QOS)
		return CLOCK_GetPllFreq(kCLOCK_SystemPll2Ctrl) / 10 /
			CLOCK_GetRootPreDivider(kCLOCK_RootEnetQosTimer) /
			CLOCK_GetRootPostDivider(kCLOCK_RootEnetQosTimer);
	else
		return 0;
}

/*
 * GPT input frequency (ipg_clk)
 */
uint32_t dev_get_gpt_ipg_freq(void *base)
{
	if (base == GPT1)
		return CLOCK_GetClockRootFreq(kCLOCK_Gpt1ClkRoot);
	else if (base == GPT2)
		return CLOCK_GetClockRootFreq(kCLOCK_Gpt2ClkRoot);
	else if (base == GPT3)
		return CLOCK_GetClockRootFreq(kCLOCK_Gpt3ClkRoot);
	else if (base == GPT4)
		return CLOCK_GetClockRootFreq(kCLOCK_Gpt4ClkRoot);
	else if (base == GPT5)
		return CLOCK_GetClockRootFreq(kCLOCK_Gpt5ClkRoot);
	else if (base == GPT6)
		return CLOCK_GetClockRootFreq(kCLOCK_Gpt6ClkRoot);
	else
		return 0;
}
