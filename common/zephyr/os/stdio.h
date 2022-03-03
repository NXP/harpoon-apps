/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_STDIO_H_
#define _ZEPHYR_STDIO_H_

#include <sys/printk.h>

#define os_printf(...)         printk(__VA_ARGS__)

#endif /* #ifndef _ZEPHYR_STDIO_H_ */
