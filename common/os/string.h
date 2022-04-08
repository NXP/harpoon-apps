/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STRING_H_
#define _COMMON_STRING_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/string.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/string.h"
#endif

#endif /* #ifndef _COMMON_STRING_H_ */
