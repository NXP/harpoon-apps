/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDIO_H_
#define _COMMON_STDIO_H_

#include "os.h"

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdio.h"
#elif defined(OS_FREERTOS)
  #include "freertos/os/stdio.h"
#endif

#endif /* #ifndef _COMMON_STDIO_H_ */

