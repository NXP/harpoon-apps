/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device interface for genAVB
 */
#ifndef _DEV_ITF_H_
#define _DEV_ITF_H_

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

#endif /* _DEV_ITF_H_ */
