/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ZEPHYR_MQUEUE_H_
#define _ZEPHYR_MQUEUE_H_

#include "os/assert.h"

#include <zephyr/kernel.h>

typedef struct k_msgq os_mqd_t;

static inline int os_mq_open(os_mqd_t *mq, const char *name,
                             uint32_t nb_items, size_t item_size)
{
	int err;

	err = k_msgq_alloc_init((struct k_msgq *)mq, item_size, nb_items);
	os_assert(!err, "Failed to create %s", name);

	return err;
}

static inline int os_mq_close(os_mqd_t *mq)
{
	return k_msgq_cleanup((struct k_msgq *)mq);
}

static inline int os_mq_send(os_mqd_t *mq, const void *item,
                             uint32_t flags, uint32_t timeout_ms)
{
	k_timeout_t t;

	if (timeout_ms == 0U) {
		t = K_NO_WAIT;
	} else if (timeout_ms == OS_QUEUE_EVENT_TIMEOUT_MAX) {
		t = K_FOREVER;
	} else {
		t = K_MSEC(timeout_ms);
	}

	return k_msgq_put((struct k_msgq *)mq, item, t);
}

static inline int os_mq_receive(os_mqd_t *mq, void *item,
                                uint32_t flags, uint32_t timeout_ms)
{
	k_timeout_t t;

	if (timeout_ms == 0U) {
		t = K_NO_WAIT;
	} else if (timeout_ms == OS_QUEUE_EVENT_TIMEOUT_MAX) {
		t = K_FOREVER;
	} else {
		t = K_MSEC(timeout_ms);
	}

	return k_msgq_get((struct k_msgq *)mq, item, t);
}

#endif /* #ifndef _ZEPHYR_MQUEUE_H_ */
