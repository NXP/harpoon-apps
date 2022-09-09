/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#ifndef _DEV_ITF_H_
#define _DEV_ITF_H_

#include "fsl_clock.h"
#include "fsl_enet_qos.h"

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
 * Enet QOS module frequency (ipg_clk)
 */
static inline uint32_t dev_get_enet_core_freq(void *base)
{
    if (base == ENET_QOS)
        return CLOCK_GetPllFreq(kCLOCK_SystemPll2Ctrl) / 8 /
            CLOCK_GetRootPreDivider(kCLOCK_RootEnetQos) /
            CLOCK_GetRootPostDivider(kCLOCK_RootEnetQos);
    else
        return 0;
}

/*
 * GPT input frequency (ipg_clk)
 */
static inline uint32_t dev_get_gpt_ipg_freq(void *base)
{
    if (base == GPT1)
        return CLOCK_GetClockRootFreq(kCLOCK_RootGpt1);
    else if (base == GPT2)
        return CLOCK_GetClockRootFreq(kCLOCK_RootGpt2);
    else if (base == GPT3)
        return CLOCK_GetClockRootFreq(kCLOCK_RootGpt3);
    else if (base == GPT4)
        return CLOCK_GetClockRootFreq(kCLOCK_RootGpt4);
    else if (base == GPT5)
        return CLOCK_GetClockRootFreq(kCLOCK_RootGpt5);
    else if (base == GPT6)
        return CLOCK_GetClockRootFreq(kCLOCK_RootGpt6);
    else
        return 0;
}

/*
 * Enet 1588 timer frequency (ipg_clk_time)
 */
static inline uint32_t dev_get_enet_1588_freq(void *base)
{
    if (base == ENET_QOS)
        return CLOCK_GetPllFreq(kCLOCK_SystemPll2Ctrl) / 10 /
            CLOCK_GetRootPreDivider(kCLOCK_RootEnetQosTimer) /
            CLOCK_GetRootPostDivider(kCLOCK_RootEnetQosTimer);
    else
        return 0;
}

#endif /* _DEV_ITF_H_ */
