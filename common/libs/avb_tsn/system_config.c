/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "system_config.h"
#include "storage.h"
#include <stdio.h>

extern struct system_config system_cfg;

int system_config_set_net(unsigned int port_id, uint8_t *hw_addr)
{
	int ret = -1;

	if (port_id >= BOARD_NUM_PORTS)
		goto exit;

	memcpy(system_cfg.net[port_id].hw_addr, hw_addr,
		sizeof(system_cfg.net[port_id].hw_addr));

	ret = 0;
exit:
	return ret;
}

struct net_config *system_config_get_net(int port_id)
{
    char port[10];

    if (port_id >= BOARD_NUM_PORTS)
        return NULL;

    snprintf(port, 10, "/port%u", port_id);
    if (storage_cd(port) == 0) {
        storage_read_mac_address("hw_addr", system_cfg.net[port_id].hw_addr);
        storage_read_ipv4_address("ip_addr", system_cfg.net[port_id].ip_addr);
        storage_read_ipv4_address("net_mask", system_cfg.net[port_id].net_mask);
        storage_read_ipv4_address("gw_addr", system_cfg.net[port_id].gw_addr);

        storage_cd("/");
    }

    return &system_cfg.net[port_id];
}

struct avb_app_config *system_config_get_avb_app(void)
{
    struct avb_app_config *config = &system_cfg.app.avb_app_config;

    if (storage_cd("/avb_app") == 0) {
        storage_read_uint("mclock_role", &config->mclock_role);

        storage_cd("/");
    }

    return config;
}
