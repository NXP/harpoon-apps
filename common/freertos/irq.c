/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"

#include "irq.h"

#define NR_IRQS 0x3FFUL

struct irq_handler {
	void (*func)(void *data);
	void *data;
};

static struct irq_handler handler[NR_IRQS];


int irq_register(int nr, void (*func)(void *data), void *data)
{
	struct irq_handler *hdlr;
	int ret = -1;

	if (nr >= NR_IRQS)
                goto exit;

	portDISABLE_INTERRUPTS();

	hdlr = &handler[nr];
	if (!hdlr)
		goto unlock;

	hdlr->func = func;
	hdlr->data = data;

	ret = 0;
unlock:
	portENABLE_INTERRUPTS();
exit:
	return ret;
}

int irq_unregister(int nr)
{
	struct irq_handler *hdlr;
	int ret = -1;

	if (nr >= NR_IRQS)
		goto exit;

	portDISABLE_INTERRUPTS();

	hdlr = &handler[nr];
	if (!hdlr)
		goto unlock;

	hdlr->func = NULL;
	hdlr->data = NULL;

	ret = 0;
unlock:
	portENABLE_INTERRUPTS();
exit:
	return ret;
}

/* Called from port FreeRTOS_IRQ_Handler */
void vApplicationIRQHandler(uint32_t ulICCIAR)
{
	struct irq_handler *hdlr = NULL;
	unsigned int nr;

	/* In the 64-bit Cortex-A RTOS port it is necessary to clear the source
	* of the interrupt BEFORE interrupts are re-enabled. */
	// TODO: ClearInterruptSource();

	/* Re-enable interrupts. */
	__asm volatile( "MSR DAIFSET, #2" ); /* was "__asm volatile( "CPSIE I" );" */

	/* The ID of the interrupt is obtained by bitwise anding the ICCIAR value
	with 0x3FF. */
	nr = ulICCIAR & NR_IRQS;

	if (nr < NR_IRQS)
		hdlr = &handler[nr];

	if (hdlr)
		hdlr->func(hdlr->data);
}
