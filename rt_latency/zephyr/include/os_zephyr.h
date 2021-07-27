/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _OS_ZEPHYR_H_
#define _OS_ZEPHYR_H_

#include <drivers/counter.h>
#include <ztest.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <cache.h>

#define os_counter_get_top_value(dev)	counter_get_top_value(dev)

#define os_counter_is_counting_up(dev)	counter_is_counting_up(dev)

#define os_counter_ticks_to_ns(dev, ticks) 	counter_ticks_to_ns(dev, ticks)

#define os_counter_get_value(dev, now)	counter_get_value(dev, now)

#define os_counter_set_channel_alarm(dev, channel, h_alarm_cfg)		\
		counter_set_channel_alarm(dev, channel, h_alarm_cfg)

#define os_counter_start(dev)	counter_start(dev)

#define os_counter_stop(dev)	counter_stop(dev)

#define os_sem_init(sem, init_count, limit)	\
		k_sem_init(sem, init_count, limit)

#define os_sem_reset(sem)	k_sem_reset(sem)

#define os_sem_give(sem)	k_sem_give(sem)

#define os_sem_take(sem, timeout)	k_sem_take(sem, timeout)

#define os_thread_resume(handler)	k_thread_resume(handler);

#define os_thread_suspend(h_thread)	k_thread_suspend(h_thread)

#define os_thread_abort(h_thread)	k_thread_abort(h_thread)

#define os_assert_true(cond, msg, ...) zassert_true(cond, msg, ##__VA_ARGS__)

#define os_assert_equal(a, b, msg, ...) zassert_equal(a, b, msg, ##__VA_ARGS__)

#define os_usleep(us)	k_usleep(us)

#define os_busy_wait(us)	k_busy_wait(us)

#define os_invd_dcache_all() sys_cache_data_all(K_CACHE_INVD)

#define os_invd_icache_all() __asm volatile ("IC IALLUIS")

#endif /* _OS_ZEPHYR_H_ */
