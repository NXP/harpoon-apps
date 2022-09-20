/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_STDLIB_H_
#define _ZEPHYR_STDLIB_H_

#include <zephyr/kernel.h>

static inline void *os_malloc(size_t size)
{
	return k_malloc(size);
}

static inline void os_free(void *ptr)
{
	k_free(ptr);
}

#endif /* #ifndef _ZEPHYR_STDLIB_H_ */
