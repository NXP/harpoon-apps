/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_SEMAPHORE_H_
#define _COMMON_SEMAPHORE_H_

#include <limits.h>
#include <stdint.h>

#define OS_SEM_TIMEOUT_MAX    UINT_MAX

/**
 *  Flags used in some of below APIs,
 *  essentially used for FreeRTOS ; ignored for Zephyr implementation
 */
#define OS_SEM_FLAGS_ISR_CONTEXT    (1 << 0)

#if defined(OS_ZEPHYR)
  #include "zephyr/os/semaphore.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/semaphore.h"
#endif

int os_sem_init(os_sem_t *sem, uint32_t init_count);
int os_sem_give(os_sem_t *sem, uint32_t flags);
int os_sem_take(os_sem_t *sem, uint32_t flags, uint32_t timeout_ms);
int os_sem_destroy(os_sem_t *sem);

#endif /* #ifndef _COMMON_SEMAPHORE_H_ */
