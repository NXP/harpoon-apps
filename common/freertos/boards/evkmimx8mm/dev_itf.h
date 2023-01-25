/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#ifndef _DEV_ITF_H_
#define _DEV_ITF_H_

#include "fsl_clock.h"

/*
 * 24MHz XTAL oscillator
 * Primary clock source for all PLLs
 */
static inline uint32_t dev_get_pll_ref_freq(void)
{
	return CLOCK_GetFreq(kCLOCK_Osc24MClk);
}

/*
 * Audio Pll tuning
 */
static inline void dev_write_audio_pll_num(uint32_t num)
{
    /* TODO */
}

static inline uint32_t dev_read_audio_pll_num(void)
{
    /* TODO */ return 0;
}

static inline uint32_t dev_read_audio_pll_denom(void)
{
    /* TODO */ return 0;
}

static inline uint32_t dev_read_audio_pll_post_div(void)
{
    /* TODO */ return 0;
}

/*
 * Enet module frequency (ipg_clk)
 */
static inline uint32_t dev_get_enet_core_freq(void *base)
{
    return CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / 3 /
        CLOCK_GetRootPreDivider(kCLOCK_RootEnetAxi) /
        CLOCK_GetRootPostDivider(kCLOCK_RootEnetAxi);
}

/*
 * Enet 1588 timer frequency (ipg_clk_time)
 */
static inline uint32_t dev_get_enet_1588_freq(void *base)
{
    return CLOCK_GetPllFreq(kCLOCK_SystemPll2Ctrl) / 10 /
        CLOCK_GetRootPreDivider(kCLOCK_RootEnetTimer) /
        CLOCK_GetRootPostDivider(kCLOCK_RootEnetTimer);
}

/*
 * GPT input frequency (ipg_clk)
 */
static inline uint32_t dev_get_gpt_ipg_freq(void *base)
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

#endif /* _DEV_ITF_H_ */
