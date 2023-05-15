/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/counter.h"

#define USEC_PER_SEC	1000000
#define NSEC_PER_SEC	1000000000

int os_counter_start(os_counter_t *dev)
{
	return dev->ops->os_counter_start(dev);
}

int os_counter_stop(const os_counter_t *dev)
{
	return dev->ops->os_counter_stop(dev);
}

int os_counter_get_value(const os_counter_t *dev, uint32_t *cnt)
{
	return dev->ops->os_counter_get_value(dev, cnt);
}

bool os_counter_is_counting_up(const os_counter_t *dev)
{
	return dev->ops->os_counter_is_counting_up(dev);
}

uint32_t os_counter_us_to_ticks(const os_counter_t *dev, uint64_t time_us)
{
	uint64_t ticks = (time_us * dev->ops->os_counter_get_frequency(dev)) / USEC_PER_SEC;

	return ticks;
}

uint64_t os_counter_ticks_to_ns(const os_counter_t *dev, uint32_t ticks)
{
	return (NSEC_PER_SEC * (uint64_t)ticks) / dev->ops->os_counter_get_frequency(dev);
}

uint32_t os_counter_get_top_value(const os_counter_t *dev)
{
	return dev->ops->os_counter_get_top_value(dev);
}

uint8_t os_counter_get_num_of_channels(const os_counter_t *dev)
{
	return dev->ops->os_counter_get_num_of_channels(dev);
}

/*
 * After expiration alarm can be set again, disabling is not needed.
 * When alarm expiration handler is called, channel is considered available and can be set again in that context.
 */
int os_counter_set_channel_alarm(os_counter_t *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm_cfg)
{
	return dev->ops->os_counter_set_channel_alarm(dev, chan_id, alarm_cfg);
}

int os_counter_cancel_channel_alarm(os_counter_t *dev, uint8_t chan_id)
{
	return dev->ops->os_counter_cancel_channel_alarm(dev, chan_id);
}
