/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)
#define tictac_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);
static void tictac_task(void *pvParameters);
extern void hello_world_doNothing(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    BaseType_t xResult;

    /* Init board cpu and hardware. */
    BOARD_InitMemory();
    BOARD_InitDebugConsole();

    xResult = xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL);
    assert(xResult == pdPASS);

    xResult = xTaskCreate(tictac_task, "Tictac_task", configMINIMAL_STACK_SIZE + 100, NULL, tictac_task_PRIORITY, NULL);
    assert(xResult == pdPASS);

    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void hello_task(void *pvParameters)
{
    for (;;)
    {
        PRINTF("\r\nHello world.\r\n");

        hello_world_doNothing();

        vTaskSuspend(NULL);
    }
}

/*!
 * @brief Task responsible for printing of "tic tac" messages.
 */
static void tictac_task(void *pvParameters)
{
    unsigned long long count = 0;
#define TIME_DELAY_SLEEP      (1 * configTICK_RATE_HZ)

    for (;;)
    {
        vTaskDelay(TIME_DELAY_SLEEP);

        if (++count % 2)
            PRINTF("tic ");
        else
            PRINTF("tac ");

        if (!(count % 20))
            PRINTF("\r\n");
    }
}
