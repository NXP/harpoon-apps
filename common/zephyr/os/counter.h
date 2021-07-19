/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_COUNTER_H_
#define _ZEPHYR_COUNTER_H_

#include <drivers/counter.h>

#define OS_COUNTER_ALARM_CFG_ABSOLUTE	COUNTER_ALARM_CFG_ABSOLUTE

#define os_counter_alarm_cfg	counter_alarm_cfg

static inline uint32_t os_counter_get_top_value(const void *dev)
{
    return counter_get_top_value((const struct device *) dev);
}

static inline bool os_counter_is_counting_up(const void *dev)
{
    return counter_is_counting_up((const struct device *) dev);
}

static inline uint64_t os_counter_ticks_to_ns(const void *dev, uint32_t ticks)
{
    return counter_ticks_to_ns((const struct device *)dev, ticks);
}

static inline uint32_t os_counter_us_to_ticks(const void *dev, uint64_t us)
{
    return counter_us_to_ticks((const struct device *)dev, us);
}

static inline int os_counter_get_value(const void *dev, uint32_t *ticks)
{
    return counter_get_value((const struct device *)dev, ticks);
}

static inline int os_counter_set_channel_alarm(const void *dev,
					uint8_t chan_id,
					const struct counter_alarm_cfg *alarm_cfg)
{
    return counter_set_channel_alarm((const struct device *)dev, chan_id, alarm_cfg);
}

static inline int os_counter_start(const void *dev)
{
    return counter_start((const struct device *)dev);
}

static inline int os_counter_stop(const void *dev)
{
    return counter_stop((const struct device *)dev);
}

#endif /* #ifndef _ZEPHYR_COUNTER_H_ */
