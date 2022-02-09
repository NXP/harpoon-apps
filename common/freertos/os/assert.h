/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_ASSERT_H_
#define _FREERTOS_ASSERT_H_

#include "os/stdio.h"

#define os_assert(cond, msg, ...)       \
do { \
    if (!(cond)) { \
        os_printf("\tAssertion failed at %s: %d: %s:\n\r" msg "\n\r", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        while(1); \
    } \
} while (0)

#define os_assert_equal(a, b, msg, ...)      os_assert((a) == (b), msg, ##__VA_ARGS__)

#endif /* #ifndef _FREERTOS_ASSERT_H_ */
