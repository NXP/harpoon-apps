/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_COUNTER_H_
#define _COMMON_COUNTER_H_

#include <stdint.h>

struct os_counter_alarm_cfg {
	void (*callback)(const void *dev, uint8_t chan_id, uint32_t irq_counter, void *user_data);
	uint32_t ticks;      /* number of ticks that triggers the alarm, relative to now */
	void    *user_data;  /* pointer to some data sent back to the callback function */
	uint32_t flags;      /* absolute or relative */
};

#if defined(OS_ZEPHYR)
  #include "zephyr/os/counter.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/counter.h"
#endif

int os_counter_start(const void *dev);
int os_counter_stop(const void *dev);
uint8_t os_counter_get_num_of_channels(const void *dev);
int os_counter_get_value(const void *dev, uint32_t *cnt);
bool os_counter_is_counting_up(const void *dev);
uint32_t os_counter_us_to_ticks(const void *dev, uint64_t period_us);
uint64_t os_counter_ticks_to_ns(const void *dev, uint32_t ticks);
uint32_t os_counter_get_top_value(const void *dev);
int os_counter_set_channel_alarm(const void *dev, uint8_t chan_id,
          const struct os_counter_alarm_cfg *alarm_cfg);

#endif /* #ifndef _COMMON_COUNTER_H_ */
