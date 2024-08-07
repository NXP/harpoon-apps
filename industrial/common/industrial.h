/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_H_
#define _INDUSTRIAL_H_

#include "hrpn_ctrl.h"
#include "industrial_entry.h"

#include <stdint.h>

struct industrial_config {
	unsigned int role; /* node_type for CAN, controller/endpoint for ethernet */
	unsigned int protocol;
	uint32_t period;
	uint8_t address[6];

	void (*event_send)(void *, uint8_t, uint8_t);
	void *event_data;

	unsigned int num_io_devices;
	unsigned int control_strategy;
	unsigned int app_mode;
};

enum industrial_event {
	EVENT_TYPE_START,
	EVENT_TYPE_IRQ,
	EVENT_TYPE_TIMER
};

void ethernet_ctrl(struct hrpn_cmd_ethernet *cmd, unsigned int len,
	       struct rpmsg_ept *ept, void *priv);

#endif /* _INDUSTRIAL_H_ */
