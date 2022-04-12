/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_CPU_LOAD_H_
#define _COMMON_CPU_LOAD_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/cpu_load.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/cpu_load.h"
#endif

#endif /* #ifndef _COMMON_CPU_LOAD_H_ */
