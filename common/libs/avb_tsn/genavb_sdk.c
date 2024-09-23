/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "genavb_sdk.h"
#include "system_config.h"

# if defined(BOARD_NUM_GPT) && (BOARD_NUM_GPT > 0)
unsigned int BOARD_GPT_clk_src(void *base)
{
	return kGPT_ClockSource_Periph;
}

unsigned int BOARD_GPT_clk_freq(void *base)
{
	return dev_get_gpt_ipg_freq(base);
}
#endif

# if defined(BOARD_NUM_TPM) && (BOARD_NUM_TPM > 0)
unsigned int BOARD_TPM_clk_src(void *base)
{
	return kTPM_SystemClock;
}

unsigned int BOARD_TPM_clk_freq(void *base)
{
	return dev_get_tpm_counter_freq(base);
}
#endif

int BOARD_NetPort_Get_MAC(unsigned int port, uint8_t *mac)
{
    const struct net_config *net_cfg = system_config_get_net(port);

    if (!net_cfg)
        return -1;

    memcpy(mac, net_cfg->hw_addr, 6);

    return 0;
}
