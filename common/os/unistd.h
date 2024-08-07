/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _COMMON_UNISTD_H_
#define _COMMON_UNISTD_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/unistd.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/unistd.h"
#endif

/*
 * Notes:
 * sleep functions may return before reaching requested timeout, return value
 * indicates remaining timeout.
 * sleep timeout has a coarse granularity (ticks), so don't use for precise
 * timeouts.
 */
int os_msleep(int32_t msec);

#endif /* #ifndef _COMMON_UNISTD_H_ */
