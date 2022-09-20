/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_ASSERT_H_
#define _ZEPHYR_ASSERT_H_

#include <zephyr/sys/__assert.h>

#define os_assert(cond, msg, ...)				\
	do {							\
		if (unlikely(!(cond))) {			\
			__ASSERT(cond, msg, ##__VA_ARGS__);	\
			while(1);				\
		}						\
	} while(0)

#define os_assert_equal(a, b, msg, ...)      os_assert((a) == (b), msg, ##__VA_ARGS__)

#endif /* #ifndef _ZEPHYR_ASSERT_H_ */
