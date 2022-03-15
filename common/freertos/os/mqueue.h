/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FREERTOS_MQUEUE_H_
#define _FREERTOS_MQUEUE_H_

#include "FreeRTOS.h"
#include "queue.h"

typedef struct {
    QueueHandle_t handle;

} os_mqd_t;

static inline int os_mq_send(os_mqd_t *mq, const void *item,
               uint32_t flags, uint32_t timeout_ms)
{
    BaseType_t ret;

    if (flags & OS_MQUEUE_FLAGS_ISR_CONTEXT) {
        BaseType_t xYieldRequired = pdFALSE;

        ret = xQueueSendToBackFromISR(mq->handle, item, &xYieldRequired);

        /* if xYieldRequired was set to true, we should yield */
        portYIELD_FROM_ISR( xYieldRequired );

    } else {
        TickType_t t;

        if (timeout_ms == OS_QUEUE_EVENT_TIMEOUT_MAX)
            t = portMAX_DELAY;
        else
            t = pdMS_TO_TICKS(timeout_ms);

        ret = xQueueSendToBack(mq->handle, item, t);
    }

    return (ret == pdTRUE) ? 0 : -1;
}

static inline int os_mq_receive(os_mqd_t *mq, void *item,
              uint32_t flags, uint32_t timeout_ms)
{
    BaseType_t ret;

    if (flags & OS_MQUEUE_FLAGS_ISR_CONTEXT) {
        BaseType_t xYieldRequired = pdFALSE;

        ret = xQueueReceiveFromISR(mq->handle, item, &xYieldRequired);

        /* if xYieldRequired was set to true, we should yield */
        portYIELD_FROM_ISR( xYieldRequired );

    } else {
        TickType_t t;

        if (timeout_ms == OS_QUEUE_EVENT_TIMEOUT_MAX)
            t = portMAX_DELAY;
        else
            t = pdMS_TO_TICKS(timeout_ms);

        ret = xQueueReceive(mq->handle, item, t);
    }

    return (ret == pdTRUE) ? 0 : -1;
}

#endif /* #ifndef _FREERTOS_MQUEUE_H_ */
