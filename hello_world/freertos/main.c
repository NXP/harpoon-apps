/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#if __has_include("clock_config.h")
#include "clock_config.h"
#endif

#include "hlog.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)
#define tictac_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_func(void *pvParameters);
static void tictac_func(void *pvParameters);
extern void hello_world_doNothing(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
__WEAK void BOARD_InitClocks(void) {}

/*!
 * @brief Application entry point.
 */
int main(void)
{
    BaseType_t xResult;

    /* Init board cpu and hardware. */
    BOARD_InitMemory();
    BOARD_InitClocks();
    BOARD_InitDebugConsole();

    xResult = xTaskCreate(hello_func, "Hello_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL);
    assert(xResult == pdPASS);

    xResult = xTaskCreate(tictac_func, "Tictac_task", configMINIMAL_STACK_SIZE + 100, NULL, tictac_task_PRIORITY, NULL);
    assert(xResult == pdPASS);

    vTaskStartScheduler();
    for (;;)
        ;

    return xResult;
}

/*!
 * @brief function responsible for printing of "Hello world." message.
 */
static void hello_func(void *pvParameters)
{
    for (;;)
    {
        log_raw(INFO, "\r\n");
        log_info("Hello world.\n");

        hello_world_doNothing();

        vTaskSuspend(NULL);
    }
}

/*!
 * @brief function responsible for printing of "tic tac" messages.
 */
static void tictac_func(void *pvParameters)
{
    unsigned long long count = 0;
#define TIME_DELAY_SLEEP      (1 * configTICK_RATE_HZ)

    for (;;)
    {
        vTaskDelay(TIME_DELAY_SLEEP);

        if (++count % 2)
            log_raw(INFO, "tic ");
        else
            log_raw(INFO, "tac ");

        if (!(count % 20))
            log_raw(INFO, "\r\n");
    }
}
