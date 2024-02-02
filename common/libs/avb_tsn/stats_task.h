/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STATS_H_
#define _STATS_H_

#include "rtos_abstraction_layer.h"

#define STATS_MAX_TASKS     18
#define ASYNC_NUM_MSG       16

#define CONFIG_STATS_CPU_LOAD       0
#define CONFIG_STATS_HEAP           0
#define CONFIG_STATS_TOTAL_CPU_LOAD 0
#define CONFIG_STATS_LWIP           0
#define CONFIG_STATS_ASYNC          1

#if CONFIG_STATS_CPU_LOAD
struct TasksCPULoad_Ctx {
    TaskStatus_t __TaskStatusArray[2][STATS_MAX_TASKS];
    TaskStatus_t *TaskStatusArray;
    TaskStatus_t *LastTaskStatusArray;
    uint32_t LastTotalTime;
    BaseType_t LastNbTasks;
};
#endif

struct Async_Msg {
    void (*Func)(void *Data);
    void *Data;
};

struct Async_Ctx {
    uint8_t qBuffer[ASYNC_NUM_MSG * sizeof(struct Async_Msg)];
    rtos_mqueue_t qObj;
};

struct StatsTask_Ctx {
#if CONFIG_STATS_CPU_LOAD
    struct TasksCPULoad_Ctx TasksCPULoad;
#endif
#if CONFIG_STATS_ASYNC
    struct Async_Ctx Async;
#endif
    void (*PeriodicFn)(void *Data);
    void *PeriodicData;
    unsigned int PeriodMs;
    rtos_thread_t stats_task;
};

int STATS_TaskInit(void (*PeriodicFn)(void *data), void *Data, unsigned int PeriodMs);
void STATS_TaskExit(void);

#if CONFIG_STATS_ASYNC
int STATS_Async(void (*Func)(void *Data), void *Data);
#else
static inline int STATS_Async(void (*Func)(void *Data), void *Data)
{
    return -1;
}
#endif

#endif /* _STATS_H_*/
