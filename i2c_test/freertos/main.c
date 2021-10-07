/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* harpoon-apps includes. */
#include "board.h"
#include "i2c_test.h"
#include "os/stdio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define test_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void test_task(void *pvParameters);

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

    xResult = xTaskCreate(test_task, "driver_test_task", configMINIMAL_STACK_SIZE + 100, NULL, test_task_PRIORITY, NULL);
    assert(xResult == pdPASS);

    vTaskStartScheduler();

    for (;;)
        ;

    return xResult;
}

static void test_task(void *pvParameters)
{
    /* Run i2c tests */
    i2c_tests();

    for (;;)
        ;
}
