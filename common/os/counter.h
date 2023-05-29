/*
 * Copyright 2021, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_COUNTER_H_
#define _COMMON_COUNTER_H_

#include "os/stdint.h"

typedef struct os_counter os_counter_t;

struct os_counter_alarm_cfg {
	void (*callback)(os_counter_t *dev, uint8_t chan_id, uint32_t irq_counter, void *user_data);
	uint32_t ticks;      /* number of ticks that triggers the alarm, relative to now */
	void    *user_data;  /* pointer to some data sent back to the callback function */
	uint32_t flags;      /* absolute or relative */
};

#if defined(OS_ZEPHYR)
  #include "zephyr/os/counter.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/counter.h"
#endif

int os_counter_start(os_counter_t *dev);
int os_counter_stop(const os_counter_t *dev);
uint8_t os_counter_get_num_of_channels(const os_counter_t *dev);
int os_counter_get_value(const os_counter_t *dev, uint32_t *cnt);
bool os_counter_is_counting_up(const os_counter_t *dev);
uint32_t os_counter_us_to_ticks(const os_counter_t *dev, uint64_t period_us);
uint64_t os_counter_ticks_to_ns(const os_counter_t *dev, uint32_t ticks);
uint32_t os_counter_get_top_value(const os_counter_t *dev);
int os_counter_set_channel_alarm(os_counter_t *dev, uint8_t chan_id,
          const struct os_counter_alarm_cfg *alarm_cfg);
int os_counter_cancel_channel_alarm(os_counter_t *dev, uint8_t chan_id);

#endif /* #ifndef _COMMON_COUNTER_H_ */
