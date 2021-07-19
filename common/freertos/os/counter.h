/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_COUNTER_H_
#define _FREERTOS_COUNTER_H_

/* System includes.*/
#include <stdbool.h>
#include <stdint.h>

/* Freescale includes. */
#include "fsl_common.h"
#include "fsl_gpt.h"

/* TODO: Use this to avoid exporting global variables */
struct os_counter_gpt_dev {
    GPT_Type *base;
    IRQn_Type *irqn;
    void *cb;
};

struct os_counter_alarm_cfg {
	void    *callback;   /* callback when "now + ticks" timer is reached */
	uint32_t ticks;      /* number of ticks that triggers the alarm, relative to now */
	void    *user_data;  /* pointer to some data sent back to the callback function */
	uint32_t flags;      /* absolute or relative */
};

#define OS_COUNTER_ALARM_CFG_ABSOLUTE (1 << 0)

void os_counter_init(const void *dev);

#endif /* #ifndef _FREERTOS_COUNTER_H_ */
