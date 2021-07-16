/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_STDIO_H_
#define _FREERTOS_STDIO_H_

#include "fsl_debug_console.h"

#define os_printf(...)         PRINTF(__VA_ARGS__)

#endif /* #ifndef _FREERTOS_STDIO_H_ */
