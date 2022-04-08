/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "log.h"
#include "sdk_printf.h"

int sdk_printf(const char *fmt_s, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt_s);
    rc = os_vprintf(fmt_s, ap);
    va_end(ap);

    return rc;
}

int sdk_vprintf(const char *fmt_s, va_list ap)
{
    return os_vprintf(fmt_s, ap);
}
