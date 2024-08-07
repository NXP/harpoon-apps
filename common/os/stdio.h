/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _COMMON_STDIO_H_
#define _COMMON_STDIO_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdio.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include <stdarg.h>
#endif

int os_printf(const char *fmt_s, ...);
int os_vprintf(const char *fmt_s, va_list ap);

#endif /* #ifndef _COMMON_STDIO_H_ */
