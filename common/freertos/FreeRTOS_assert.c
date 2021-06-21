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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void vMainAssertCalled( const char * pcFileName, uint32_t ulLineNumber )
{
    PRINTF( "ASSERT!  Line %lu of file %s\r\n", ulLineNumber, pcFileName);

    taskENTER_CRITICAL();

    for( ;; );

    taskEXIT_CRITICAL();
}
