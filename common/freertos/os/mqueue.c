/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/mqueue.h"

#include "rtos_abstraction_layer.h"

int os_mq_open(os_mqd_t *mq, const char *name, uint32_t nb_items, size_t item_size)
{
    mq->handle = xQueueCreate(nb_items, item_size);
    rtos_assert(mq->handle, "Failed to create %s", name);

    return (mq->handle == NULL) ? -1 : 0;
}

int os_mq_close(os_mqd_t *mq)
{
    vQueueDelete(mq->handle);

    return 0;
}
