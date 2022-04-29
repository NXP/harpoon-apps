/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _LOGGING_SDK_PRINTF_H_
#define _LOGGING_SDK_PRINTF_H_

#include <stdarg.h>

int sdk_printf(const char *fmt_s, ...);
int sdk_vprintf(const char *fmt_s, va_list ap);

#endif /* _LOGGING_SDK_PRINTF_H_ */
