/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_SEMAPHORE_H_
#define _ZEPHYR_SEMAPHORE_H_

#include <zephyr/kernel.h>

typedef struct k_sem os_sem_t;

static inline int os_sem_init(os_sem_t *sem, uint32_t init_count)
{
    return k_sem_init((struct k_sem *)sem, init_count, UINT_MAX);
}

static inline int os_sem_destroy(os_sem_t *sem)
{
    return 0;
}

static inline int os_sem_give(os_sem_t *sem, uint32_t flags)
{
    k_sem_give((struct k_sem *)sem);

    return 0;
}

static inline int os_sem_take(os_sem_t *sem, uint32_t flags, uint32_t timeout_ms)
{
    return k_sem_take((struct k_sem *)sem, (timeout_ms == OS_SEM_TIMEOUT_MAX) ?
                     K_FOREVER : K_MSEC(timeout_ms));
}

#endif /* #ifndef _ZEPHYR_SEMAPHORE_H_ */
