/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDINT_H_
#define _COMMON_STDINT_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdint.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/stdint.h"
#endif

#endif /* #ifndef _COMMON_STDINT_H_ */
