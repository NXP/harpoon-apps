/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_ETHERNET_H_
#define _INDUSTRIAL_ETHERNET_H_

#include "hrpn_ctrl.h"
#include "industrial.h"

struct ethernet_ctx {
	void (*event_send)(void *, uint8_t, uint8_t);
	void *event_data;

	uint8_t mac_addr[6];
	uint32_t period;
	uint32_t role;
	bool loopback;

	uint32_t num_io_devices;
	uint32_t control_strategy;

	uint32_t app_mode;
};

#endif /* _INDUSTRIAL_ETHERNET_H_ */
