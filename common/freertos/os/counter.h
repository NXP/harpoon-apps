/*
 * Copyright 2021, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_COUNTER_H_
#define _FREERTOS_COUNTER_H_

/* OSAL includes.*/
#include "os/counter.h"
#include "os/irq.h"

/* TODO: support multiple channels */
#define NB_CHANNELS		1

struct os_counter_ops {
	int (*os_counter_start)(os_counter_t *dev);
	int (*os_counter_stop)(const os_counter_t *dev);
	uint8_t (*os_counter_get_num_of_channels)(const os_counter_t *dev);
	int (*os_counter_get_value)(const os_counter_t *dev, uint32_t *cnt);
	bool (*os_counter_is_counting_up)(const os_counter_t *dev);
	uint32_t (*os_counter_us_to_ticks)(const os_counter_t *dev, uint64_t period_us);
	uint64_t (*os_counter_ticks_to_ns)(const os_counter_t *dev, uint32_t ticks);
	uint32_t (*os_counter_get_top_value)(const os_counter_t *dev);
	int (*os_counter_set_channel_alarm)(os_counter_t *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm_cfg);
	int (*os_counter_cancel_channel_alarm)(os_counter_t *dev, uint8_t chan_id);
};

struct os_counter {
	void *base;
	IRQn_Type irqn;
	bool initialized; /* True if HW is initialized */
	const struct os_counter_ops *ops; //the hardware specific counter ops
	struct os_counter_alarm_cfg alarms[NB_CHANNELS];
};

#define OS_COUNTER_ALARM_CFG_ABSOLUTE (1 << 0)

extern os_counter_t freertos_counter_instance_0;
extern os_counter_t freertos_counter_instance_1;

#define GET_COUNTER_DEVICE_INSTANCE(inst)      (&freertos_counter_instance_ ## inst)

#endif /* #ifndef _FREERTOS_COUNTER_H_ */
