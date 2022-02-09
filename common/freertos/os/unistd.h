/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_UNISTD_H_
#define _FREERTOS_UNISTD_H_

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

static inline int os_msleep(int32_t msec)
{
    const TickType_t xDelay = pdMS_TO_TICKS(msec);

    vTaskDelay(xDelay);

    return 0;
}

#endif /* #ifndef _FREERTOS_UNISTD_H_ */
