/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2021-2022 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* NXP includes. */
#include "fsl_device_registers.h"
#include "fsl_common.h"

#include "irq.h"

/*******************************************************************************
 * Global variables
 ******************************************************************************/

/*  computed once for all,  used to program the timer */
uint32_t FreeRTOS_tick_interval;

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ARM_TIMER            ARM_TIMER_VIRTUAL

static void VirtualTimer_IRQn_Handler(void *data)
{
	(void)data;
	FreeRTOS_Tick_Handler();
}

/*-----------------------------------------------------------*/

/*
 * Use the generic EL1 virtual timer for the tick interrupt.
 *
 * Note from ARM documentation[1]:
 *
 * When software writes TVAL, the processor reads the current system
 * count internally, adds the written value, and then populates CVAL:
 *
 *  CVAL = TVAL + System Counter
 *  Timer Condition Met: CVAL <= System Count
 *
 * [1] https://developer.arm.com/documentation/102379/0000/The-processor-timers
 */

void vConfigureTickInterrupt( void )
{
	uint32_t val;

	/* Initialize the device. */
	ARM_TIMER_Initialize(ARM_TIMER);

	/* Fetch timer frequency rate. */
	ARM_TIMER_GetFreq(&val);

	/* Derive values from the tick rate. */
	FreeRTOS_tick_interval = val / configTICK_RATE_HZ;

	/* Set the timer tick interval. */
	ARM_TIMER_SetInterval(ARM_TIMER, FreeRTOS_tick_interval);

	irq_register(VirtualTimer_IRQn, VirtualTimer_IRQn_Handler, NULL, portLOWEST_USABLE_INTERRUPT_PRIORITY);

	/* Enable the interrupt in the GIC. */
	GIC_EnableInterface();
	GIC_SetInterfacePriorityMask(portLOWEST_INTERRUPT_PRIORITY << portPRIORITY_SHIFT);
	GIC_EnableIRQ(VirtualTimer_IRQn);

	/* Start the timer with interrupt enabled. */
	ARM_TIMER_Start(ARM_TIMER, true);
}
/*-----------------------------------------------------------*/

/*
 * The interrupts generated by the timer behave in a level-sensitive manner.
 * This means that, once the timer firing condition is reached, the timer
 * will continue to signal an interrupt until one of the following
 * situations occurs:
 *
 * o IMASK is set to one, which masks the interrupt.
 * o ENABLE is cleared to 0, which disables the timer.
 * o TVAL or CVAL is written, so that firing condition is no longer met.
 *
 * [Ref] https://developer.arm.com/documentation/102379/0000/The-processor-timers
 */
void vClearTickInterrupt( void )
{
	/* Set the timer tick interval. */
	ARM_TIMER_SetInterval(ARM_TIMER, FreeRTOS_tick_interval);
}
