/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_ASSERT_H_
#define _ZEPHYR_ASSERT_H_

#define os_assert(cond, msg, ...) zassert_true(cond, msg, ##__VA_ARGS__)

#define os_assert_equal(a, b, msg, ...) zassert_equal(a, b, msg, ##__VA_ARGS__)

#endif /* #ifndef _ZEPHYR_ASSERT_H_ */
