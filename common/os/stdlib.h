/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_STDLIB_H_
#define _COMMON_STDLIB_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/stdlib.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/stdlib.h"
#endif

void *os_malloc(size_t size);
void os_free(void *ptr);

#endif /* #ifndef _COMMON_STDLIB_H_ */
