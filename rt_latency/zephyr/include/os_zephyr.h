/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _OS_ZEPHYR_H_
#define _OS_ZEPHYR_H_

#include <ztest.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <cache.h>

#define os_thread_resume(handler)	k_thread_resume(handler);

#define os_thread_suspend(h_thread)	k_thread_suspend(h_thread)

#define os_thread_abort(h_thread)	k_thread_abort(h_thread)

#define os_usleep(us)	k_usleep(us)

#define os_busy_wait(us)	k_busy_wait(us)

#endif /* _OS_ZEPHYR_H_ */
