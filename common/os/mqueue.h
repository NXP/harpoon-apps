/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_MQUEUE_H
#define _COMMON_MQUEUE_H

#include "os/limits.h"
#include "os/stdint.h"

#define OS_QUEUE_EVENT_TIMEOUT_MAX    UINT_MAX

/**
 *  Flags used in some of below APIs
 */
#define OS_MQUEUE_FLAGS_ISR_CONTEXT    (1 << 0)

#if defined(OS_ZEPHYR)
  #include "zephyr/os/mqueue.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/mqueue.h"
#endif

int os_mq_open(os_mqd_t *mq, const char *name, uint32_t nb_items, size_t item_size);
int os_mq_send(os_mqd_t *mq, const void *item, uint32_t flags, uint32_t timeout_ms);
int os_mq_receive(os_mqd_t *mq, void *item, uint32_t flags, uint32_t timeout_ms);
int os_mq_close(os_mqd_t *mq);

#endif /* #ifndef _COMMON_MQUEUE_H */
