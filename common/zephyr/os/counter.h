/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_COUNTER_H_
#define _ZEPHYR_COUNTER_H_

#include <zephyr/drivers/counter.h>

#include "os/assert.h"
#include "os/counter.h"

struct os_counter {
	struct device dev;
};

#define OS_COUNTER_ALARM_CFG_ABSOLUTE	COUNTER_ALARM_CFG_ABSOLUTE

static inline uint32_t os_counter_get_top_value(const os_counter_t *dev)
{
	return counter_get_top_value((const struct device *)dev);
}

static inline bool os_counter_is_counting_up(const os_counter_t *dev)
{
	return counter_is_counting_up((const struct device *)dev);
}

static inline uint64_t os_counter_ticks_to_ns(const os_counter_t *dev, uint32_t ticks)
{
	return counter_ticks_to_ns((const struct device *)dev, ticks);
}

static inline uint32_t os_counter_us_to_ticks(const os_counter_t *dev, uint64_t us)
{
	return counter_us_to_ticks((const struct device *)dev, us);
}

static inline int os_counter_get_value(const os_counter_t *dev, uint32_t *ticks)
{
	return counter_get_value((const struct device *)dev, ticks);
}

static inline int os_counter_set_channel_alarm(os_counter_t *dev, uint8_t chan_id,
		  const struct os_counter_alarm_cfg *alarm_cfg)
{
	struct counter_alarm_cfg a;

	os_assert(alarm_cfg != NULL, "Null pointer!");

	a.callback = (void (*)(const struct device *, uint8_t, uint32_t, void *)) alarm_cfg->callback;
	a.ticks = alarm_cfg->ticks;
	a.user_data = alarm_cfg->user_data;
	a.flags = alarm_cfg->flags;

	return counter_set_channel_alarm((const struct device *)dev, chan_id, &a);
}

static inline int os_counter_cancel_channel_alarm(os_counter_t *dev, uint8_t chan_id)
{
	return counter_cancel_channel_alarm((const struct device *)dev, chan_id);
}

static inline int os_counter_start(os_counter_t *dev)
{
	return counter_start((const struct device *)dev);
}

static inline int os_counter_stop(const os_counter_t *dev)
{
	return counter_stop((const struct device *)dev);
}

#endif /* #ifndef _ZEPHYR_COUNTER_H_ */
