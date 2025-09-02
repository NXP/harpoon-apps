/*
 * Copyright 2018, 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "log.h"

#include "rtos_abstraction_layer.h"
#include "stats_task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STATS_TASK_NAME       "stats"
#define STATS_TASK_STACK_SIZE (RTOS_MINIMAL_STACK_SIZE + 185)
#define STATS_TASK_PRIORITY   5

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern unsigned int malloc_failed_count;
extern uint32_t idleCounter;

static struct StatsTask_Ctx StatsTask;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if CONFIG_STATS_HEAP
static void STATS_Heap(void)
{
    size_t FreeHeap, UsedHeap;

    FreeHeap = xPortGetFreeHeapSize();
    UsedHeap = configTOTAL_HEAP_SIZE - FreeHeap;

    rtos_printf("Heap used: %u, free: %u, min free: %u\n",
           UsedHeap, FreeHeap,
           xPortGetMinimumEverFreeHeapSize());

    rtos_printf("Malloc failed counter: %u\n\n", malloc_failed_count);
}
#endif

#if CONFIG_STATS_CPU_LOAD
static TaskStatus_t *STATS_FindTask(TaskStatus_t *TaskStatusArray,
                                    BaseType_t NbTasks, BaseType_t TaskNumber)
{
    int i;

    for (i = 0; i < NbTasks; i++) {
        if (TaskStatusArray[i].xTaskNumber == TaskNumber)
            return &TaskStatusArray[i];
    }
    return NULL;
}

static int STATS_TasksCPULoadInit(struct TasksCPULoad_Ctx *Ctx)
{
    memset(Ctx->__TaskStatusArray, 0, sizeof(TaskStatus_t) * STATS_MAX_TASKS * 2);

    Ctx->TaskStatusArray = Ctx->__TaskStatusArray[0];
    Ctx->LastTaskStatusArray = Ctx->__TaskStatusArray[1];

    return 0;
}

static void STATS_TasksCPULoad(struct TasksCPULoad_Ctx *Ctx)
{
    int i;
    TaskStatus_t *TaskStatusArray = Ctx->TaskStatusArray;
    TaskStatus_t *LastTaskStatusArray = Ctx->LastTaskStatusArray;
    BaseType_t NbTasks;
    uint32_t TotalTime, DeltaTotalTime, TotalTaskTime = 0;
    double PercentUsage;

    NbTasks = uxTaskGetSystemState(Ctx->TaskStatusArray,
                                   STATS_MAX_TASKS, &TotalTime);
    if (!NbTasks) {
        log_err("uxTaskGetSystemState error, check STATS_MAX_TASKS\n");
        return;
    }

    DeltaTotalTime = TotalTime - Ctx->LastTotalTime;

    rtos_printf("Name          Prio   %%CPU    MinStackFree\n");

    for (i = 0; i < NbTasks; i++) {
        TaskStatus_t *CurrentTaskStatus, *LastTaskStatus;

        CurrentTaskStatus = &TaskStatusArray[i];
        LastTaskStatus = STATS_FindTask(LastTaskStatusArray,
                                        Ctx->LastNbTasks,
                                        CurrentTaskStatus->xTaskNumber);
        if (LastTaskStatus) {
            uint32_t DeltaTaskTime;

            DeltaTaskTime = CurrentTaskStatus->ulRunTimeCounter - LastTaskStatus->ulRunTimeCounter;
            TotalTaskTime += DeltaTaskTime;
            PercentUsage = (double)(DeltaTaskTime * 100) / (double)DeltaTotalTime;

            rtos_printf("%-13s %d     %5.2f    %u\n",
                   CurrentTaskStatus->pcTaskName,
                   CurrentTaskStatus->uxCurrentPriority,
                   PercentUsage,
                   CurrentTaskStatus->usStackHighWaterMark);
        } else {
            rtos_printf("%-13s %d     ---\n",
                   CurrentTaskStatus->pcTaskName,
                   CurrentTaskStatus->uxCurrentPriority);
        }
    }

    if (TotalTaskTime > DeltaTotalTime)
        PercentUsage = 0;
    else
        PercentUsage = (double)((DeltaTotalTime - TotalTaskTime) * 100) / (double)DeltaTotalTime;

    rtos_printf("%-13s %d     %5.2f\n", "other", 0, PercentUsage);

    Ctx->LastTaskStatusArray = TaskStatusArray;
    Ctx->TaskStatusArray = LastTaskStatusArray;
    Ctx->LastTotalTime = TotalTime;
    Ctx->LastNbTasks = NbTasks;
}
#endif

#if CONFIG_STATS_ASYNC
static int STATS_AsyncInit(struct Async_Ctx *Ctx)
{
    if (rtos_mqueue_init(&Ctx->qObj, ASYNC_NUM_MSG, sizeof(struct Async_Msg),
                                      Ctx->qBuffer) < 0)
        goto err;

    return 0;

err:
    return -1;
}

int STATS_Async(void (*Func)(void *Data), void *Data)
{
    struct Async_Ctx *Ctx = &StatsTask.Async;
    struct Async_Msg Msg;

    Msg.Func = Func;
    Msg.Data = Data;

    return rtos_mqueue_send(&Ctx->qObj, &Msg, RTOS_NO_WAIT);
}

static void STATS_AsyncProcess(struct Async_Ctx *Ctx, unsigned int WaitMs)
{
    struct Async_Msg Msg;
    unsigned int Last, Now;
    unsigned int Elapsed, Timeout;

    Timeout = RTOS_TICKS_TO_UINT(RTOS_MS_TO_TICKS(WaitMs));
    Last = rtos_get_current_time();

    while (true) {
        if (rtos_mqueue_receive(&Ctx->qObj, &Msg, RTOS_UINT_TO_TICKS(Timeout)) == 0) {
            Msg.Func(Msg.Data);

            Now = rtos_get_current_time();
            Elapsed = Now - Last;

            if (Elapsed < Timeout) {
                Timeout -= Elapsed;
                Last = Now;
            } else
                break;
        } else
            break;
    }
}
#endif

#if CONFIG_STATS_TOTAL_CPU_LOAD
static void STATS_TotalCPULoad(unsigned int periodMs)
{
    static uint32_t lastIdleCounter = 0;
    uint32_t idleCnt = idleCounter;
    float cpu_load = 100.0 - ((idleCnt - lastIdleCounter) / (BOARD_IDLE_COUNT_PER_S * (periodMs / 1000.0))) * 100.0;

    rtos_printf("Total CPU load : %5.2f\n\n", cpu_load);

    lastIdleCounter = idleCnt;
}
#endif

static void STATS_TaskPeriodic(struct StatsTask_Ctx *Ctx)
{
    if (Ctx->PeriodicFn)
        Ctx->PeriodicFn(Ctx->PeriodicData);

#if CONFIG_STATS_CPU_LOAD
    STATS_TasksCPULoad(&Ctx->TasksCPULoad);
#endif
#if CONFIG_STATS_HEAP
    STATS_Heap();
#endif
#if CONFIG_STATS_TOTAL_CPU_LOAD
    STATS_TotalCPULoad(Ctx->PeriodMs);
#endif
#if CONFIG_STATS_LWIP
    stats_display();
#endif
}

static void STATS_Task(void *pvParameters)
{
    struct StatsTask_Ctx *Ctx = pvParameters;

    while (true) {
#if CONFIG_STATS_ASYNC
        STATS_AsyncProcess(&Ctx->Async, Ctx->PeriodMs);
#else
        rtos_sleep(RTOS_MS_TO_TICKS(Ctx->PeriodMs));
#endif
        STATS_TaskPeriodic(Ctx);
    }
}

int STATS_TaskInit(void (*PeriodicFn)(void *Data), void *Data, unsigned int PeriodMs)
{
    struct StatsTask_Ctx *Ctx = &StatsTask;

    Ctx->PeriodicFn = PeriodicFn;
    Ctx->PeriodicData = Data;
    Ctx->PeriodMs = PeriodMs;

#if CONFIG_STATS_CPU_LOAD
    if (STATS_TasksCPULoadInit(&Ctx->TasksCPULoad) < 0)
        log_err("STATS_TasksCPULoadInit failed\n");
#endif

#if CONFIG_STATS_ASYNC
    if (STATS_AsyncInit(&Ctx->Async) < 0)
        log_err("STATS_AsyncInit failed \n");
#endif

    if (rtos_thread_create(&Ctx->stats_task, STATS_TASK_PRIORITY, 0, STATS_TASK_STACK_SIZE, STATS_TASK_NAME, STATS_Task, Ctx) < 0) {
        log_err("rtos_thread_create(%s) failed\n", STATS_TASK_NAME);
        goto exit;
    }

    return 0;

exit:
    return -1;
}

void STATS_TaskExit()
{
    struct StatsTask_Ctx *Ctx = &StatsTask;

    rtos_thread_abort(&Ctx->stats_task);
}
