/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_STDLIB_H_
#define _FREERTOS_STDLIB_H_

#include "FreeRTOS.h"

#define os_malloc(x)	pvPortMalloc(x)
#define os_free(x)	vPortFree(x)

#endif /* #ifndef _FREERTOS_STDLIB_H_ */
