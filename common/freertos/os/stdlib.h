/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_STDLIB_H_
#define _FREERTOS_STDLIB_H_

#include "FreeRTOS.h"

static inline void *os_malloc(size_t size)
{
	return pvPortMalloc(size);
}

static inline void os_free(void *ptr)
{
	vPortFree(ptr);
}

#endif /* #ifndef _FREERTOS_STDLIB_H_ */
