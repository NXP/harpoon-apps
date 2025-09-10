/*
 * Copyright 2018, 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STATS_H_
#define _STATS_H_

#include "rtos_apps/async.h"

int STATS_TaskInit(void (*PeriodicFn)(void *data), void *Data, unsigned int PeriodMs,
                         struct rtos_apps_async **async);
void STATS_TaskExit(struct rtos_apps_async *async);

#endif /* _STATS_H_*/
