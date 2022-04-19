/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_STDIO_H_
#define _ZEPHYR_STDIO_H_

#include <sys/printk.h>

#define os_printf(...)         printk(__VA_ARGS__)
#define os_vprintf(fmt, ap)    vprintk(fmt, ap)

#endif /* #ifndef _ZEPHYR_STDIO_H_ */
