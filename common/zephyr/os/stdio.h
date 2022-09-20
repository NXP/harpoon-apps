/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_STDIO_H_
#define _ZEPHYR_STDIO_H_

#include <zephyr/sys/printk.h>

static inline int os_printf(const char *fmt_s, ...)
{
    va_list ap;

    va_start(ap, fmt_s);
    vprintk(fmt_s, ap);
    va_end(ap);

    return 0;
}

static inline int os_vprintf(const char *fmt_s, va_list ap)
{
    vprintk(fmt_s, ap);

    return 0;
}

#endif /* #ifndef _ZEPHYR_STDIO_H_ */
