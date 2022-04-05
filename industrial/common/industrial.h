/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_H_
#define _INDUSTRIAL_H_

#include "industrial_entry.h"

#include <stdint.h>

struct industrial_config {

	void (*event_send)(void *, uint8_t);
	void *event_data;
};

enum industrial_event {
	EVENT_TYPE_TX_RX,
	EVENT_TYPE_START
};

#endif /* _INDUSTRIAL_H_ */
