/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_LIMITS_H_
#define _COMMON_LIMITS_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/limits.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/limits.h"
#endif

#endif /* #ifndef _COMMON_LIMITS_H_ */
