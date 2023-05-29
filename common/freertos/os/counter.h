/*
 * Copyright 2021, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_COUNTER_H_
#define _FREERTOS_COUNTER_H_

/* OSAL includes.*/
#include "os/counter.h"

/* Freescale includes. */
#include "fsl_common.h"
#include "fsl_gpt.h"

/* TODO: support multiple channels */
#define NB_CHANNELS		1

struct os_counter {
	void *base;
	IRQn_Type irqn;
	bool initialized; /* True if HW is initialized */
	struct os_counter_alarm_cfg alarms[NB_CHANNELS];
};

#define OS_COUNTER_ALARM_CFG_ABSOLUTE (1 << 0)

extern os_counter_t freertos_counter_instance_0;
extern os_counter_t freertos_counter_instance_1;

#define GET_COUNTER_DEVICE_INSTANCE(inst)      (&freertos_counter_instance_ ## inst)

#endif /* #ifndef _FREERTOS_COUNTER_H_ */
