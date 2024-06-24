/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _COMMON_ASSERT_H_
#define _COMMON_ASSERT_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/assert.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/assert.h"
#endif

#endif /* #ifndef _COMMON_ASSERT_H_ */
