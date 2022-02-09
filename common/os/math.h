/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_MATH_H_
#define _COMMON_MATH_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/math.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/math.h"
#endif

#endif /* #ifndef _COMMON_MATH_H_ */
