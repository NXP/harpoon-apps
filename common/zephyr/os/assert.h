/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_ASSERT_H_
#define _ZEPHYR_ASSERT_H_

#include <sys/__assert.h>

#define os_assert(cond, msg, ...) __ASSERT(cond, msg, ##__VA_ARGS__)

#define os_assert_equal(a, b, msg, ...) __ASSERT((a) == (b), msg, ##__VA_ARGS__)

#endif /* #ifndef _ZEPHYR_ASSERT_H_ */
