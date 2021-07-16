/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os.h"

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdio.h"
#elif defined(OS_FREERTOS)
#include "freertos/os/stdio.h"
#endif

int os_printf(const char *format, ...);
