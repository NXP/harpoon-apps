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
    return CLOCK_GetFreq(kCLOCK_IpgClk);
}

/*
 * Enet 1588 timer frequency (ipg_clk_time)
 */
static inline uint32_t dev_get_enet_1588_freq(void *base)
{
    return CLOCK_GetFreq(kCLOCK_EnetIpgClk);
}

/*
 * GPT input frequency (ipg_clk)
 */
static inline uint32_t dev_get_gpt_ipg_freq(void *base)
{
    return CLOCK_GetFreq(kCLOCK_PerClk);
}

#endif /* _DEV_ITF_H_ */
