/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDDEF_H_
#define _COMMON_STDDEF_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stddef.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/stddef.h"
#endif

#endif /* #ifndef _COMMON_STDDEF_H_ */
