/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDIO_H_
#define _COMMON_STDIO_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdio.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/stdio.h"
#endif

#endif /* #ifndef _COMMON_STDIO_H_ */
