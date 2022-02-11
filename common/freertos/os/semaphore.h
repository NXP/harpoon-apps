/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_SEMAPHORE_H_
#define _FREERTOS_SEMAPHORE_H_

#include <stdint.h>

#include "os/assert.h"

#include "semphr.h"

typedef struct {
    SemaphoreHandle_t handle;
    StaticSemaphore_t buffer;
} os_sem_t;

static inline int os_sem_init(os_sem_t *sem, uint32_t init_count)
{
    /*
     * In order to avoid dynamic allocation, the address of the semaphore buffer
     * is passed in and will be used to hold the semaphore structure.
     */
    sem->handle = xSemaphoreCreateCountingStatic((UBaseType_t)UINT_MAX,
                              (UBaseType_t) init_count, &sem->buffer);

    return (sem->handle == NULL) ? -1 : 0;
}

static inline int os_sem_destroy(os_sem_t *sem)
{
    vSemaphoreDelete(sem->handle);

    return 0;
}

static inline int os_sem_give(os_sem_t *sem, uint32_t flags)
{
    BaseType_t ret;

    if (flags & OS_SEM_FLAGS_ISR_CONTEXT) {
        BaseType_t xYieldRequired = pdFALSE;

        ret = xSemaphoreGiveFromISR(sem->handle, &xYieldRequired);

        /* If xYieldRequired was set to true, we should yield */
        portYIELD_FROM_ISR( xYieldRequired );

    } else {
        ret = xSemaphoreGive(sem->handle);
    }

    return (ret == pdTRUE) ? 0 : -1;
}

static inline int os_sem_take(os_sem_t *sem, uint32_t flags, uint32_t timeout_ms)
{
    BaseType_t ret;

    if (flags & OS_SEM_FLAGS_ISR_CONTEXT) {
        BaseType_t xYieldRequired = pdFALSE;

        ret = xSemaphoreTakeFromISR(sem->handle, &xYieldRequired);

        /* If xYieldRequired was set to true, we should yield */
        portYIELD_FROM_ISR( xYieldRequired );

    } else {
        TickType_t t;

        if (timeout_ms == OS_SEM_TIMEOUT_MAX)
            t = portMAX_DELAY;
        else
            t = pdMS_TO_TICKS(timeout_ms);

        ret = xSemaphoreTake(sem->handle, t);
    }

    return (ret == pdTRUE) ? 0 : -1;
}

#endif /* #ifndef _FREERTOS_SEMAPHORE_H_ */
