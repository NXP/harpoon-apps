/*
 * Copyright 2019, 2022-2024 NXP
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

struct net_config *system_config_get_net(unsigned int port_id)
{
	char port[10];

	if (port_id >= BOARD_NUM_PORTS)
		return NULL;

	snprintf(port, 10, "/port%u", port_id);
	if (storage_cd(port, true) == 0) {
		storage_read_mac_address("hw_addr", system_cfg.net[port_id].hw_addr);
		storage_read_ipv4_address("ip_addr", system_cfg.net[port_id].ip_addr);
		storage_read_ipv4_address("net_mask", system_cfg.net[port_id].net_mask);
		storage_read_ipv4_address("gw_addr", system_cfg.net[port_id].gw_addr);

		storage_cd("/", true);
	}

	return &system_cfg.net[port_id];
}

struct avb_app_config *system_config_get_avb_app(void)
{
	struct avb_app_config *config = &system_cfg.app.avb_app_config;

	if (storage_cd("/avb_app", true) == 0) {
		storage_read_uint("mclock_role", &config->mclock_role);

		storage_cd("/", true);
	}

	return config;
}

#if (CONFIG_GENAVB_USE_AVDECC == 1)
void system_config_set_avdecc(aem_entity_id_t id, bool milan_mode)
{
	system_cfg.avdecc.milan_mode = milan_mode;
	system_cfg.avdecc.aem_id = id;
}

struct avb_avdecc_config *system_config_get_avdecc(void)
{
	struct avb_avdecc_config *config = &system_cfg.avdecc;

	if (storage_cd("/avdecc", true) == 0) {
		storage_read_uint("aem_id", &config->aem_id);

		storage_cd("/", true);
	}

	return config;
}
#endif
