/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDLIB_H_
#define _COMMON_STDLIB_H_

#include "os.h"

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdlib.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/stdlib.h"
#endif

#endif /* #ifndef _COMMON_STDLIB_H_ */
