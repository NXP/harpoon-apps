/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_ASSERT_H_
#define _FREERTOS_ASSERT_H_

#include "FreeRTOS.h"
#include "os/stdio.h"

#define os_assert(cond, msg, ...)       \
do { \
    if (!(cond)) { \
        os_printf("%s: " msg "\n\r", __func__, ##__VA_ARGS__); \
        assert(false); \
    } \
} while (0)

#define os_assert_equal(a, b, msg, ...)      os_assert((a) == (b), msg, ##__VA_ARGS__)

#endif /* #ifndef _FREERTOS_ASSERT_H_ */
