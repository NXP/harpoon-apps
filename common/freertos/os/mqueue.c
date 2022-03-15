/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/mqueue.h"

int os_mq_open(os_mqd_t *mq, const char *name, uint32_t nb_items, size_t item_size)
{
    mq->handle = xQueueCreate(nb_items, item_size);
    os_assert(mq->handle, "Failed to create %s", name);

    return (mq->handle == NULL) ? -1 : 0;
}

int os_mq_close(os_mqd_t *mq)
{
    vQueueDelete(mq->handle);

    return 0;
}
