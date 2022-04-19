/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_STDIO_H_
#define _FREERTOS_STDIO_H_

#include "fsl_debug_console.h"

#define os_printf(...)         DbgConsole_Printf(__VA_ARGS__)
#define os_vprintf(fmt, ap)    DbgConsole_Vprintf(fmt, ap)

#endif /* #ifndef _FREERTOS_STDIO_H_ */
