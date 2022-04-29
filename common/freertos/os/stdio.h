/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_STDIO_H_
#define _FREERTOS_STDIO_H_

#include "fsl_debug_console.h"

static inline int os_printf(const char *fmt_s, ...)
{
    int rc;
    va_list ap;

    va_start(ap, fmt_s);
    rc = DbgConsole_Vprintf(fmt_s, ap);
    va_end(ap);

    return rc;
}

static inline int os_vprintf(const char *fmt_s, va_list ap)
{
    return DbgConsole_Vprintf(fmt_s, ap);
}

#endif /* #ifndef _FREERTOS_STDIO_H_ */
