/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "dev_itf.h"
#include "system_config.h"

unsigned int BOARD_GPT_clk_src(void *base)
{
#ifdef BOARD_GPT_REC_BASE
    if (base == BOARD_GPT_REC_BASE)
        return BOARD_GPT_REC_CLK_SOURCE_TYPE;
    else
#endif
        return kGPT_ClockSource_Periph;
}

unsigned int BOARD_GPT_clk_freq(void *base)
{
#ifdef BOARD_GPT_REC_BASE
    if (base == BOARD_GPT_REC_BASE)
        return BOARD_GPT_REC_CLK_EXT_FREQ;
    else
#endif
        return dev_get_gpt_ipg_freq(base);
}

int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac)
{
    const struct net_config *net_cfg = system_config_get_net(port);

    if (!net_cfg)
        return -1;

    memcpy(mac, net_cfg->hw_addr, 6);

    return 0;
}
