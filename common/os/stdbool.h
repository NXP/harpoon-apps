/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDBOOL_H_
#define _COMMON_STDBOOL_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdbool.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/stdbool.h"
#endif

#endif /* #ifndef _COMMON_STDBOOL_H_ */
