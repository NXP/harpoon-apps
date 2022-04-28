/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_ETHERNET_H_
#define _INDUSTRIAL_ETHERNET_H_

#include "hrpn_ctrl.h"
#include "industrial.h"

struct ethernet_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;

	uint8_t hw_addr[6];
};

#endif /* _INDUSTRIAL_ETHERNET_H_ */
