/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/irq.h"
#include "os/stddef.h"
#include "os/stdint.h"

#include "FreeRTOS.h"

#include "irq.h"

#define NR_IRQS 0x3FFUL

struct irq_handler {
	void (*func)(void *data);
	void *data;
};

static struct irq_handler handler[NR_IRQS];


int irq_register(int nr, void (*func)(void *data), void *data, unsigned int prio)
{
	struct irq_handler *hdlr;
	int ret = -1;

	if (nr >= NR_IRQS)
		goto exit;

	if (prio < OS_IRQ_PRIO_MAX || prio > OS_IRQ_PRIO_MIN)
		goto exit;

	portDISABLE_INTERRUPTS();

	hdlr = &handler[nr];

	hdlr->func = func;
	hdlr->data = data;

	GIC_SetPriority(nr, prio << portPRIORITY_SHIFT);

	ret = 0;

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

	hdlr->func = NULL;
	hdlr->data = NULL;

	GIC_SetPriority(nr, OS_IRQ_PRIO_DEFAULT << portPRIORITY_SHIFT);

	ret = 0;

	portENABLE_INTERRUPTS();
exit:
	return ret;
}

/* Called from port FreeRTOS_IRQ_Handler */
void vApplicationIRQHandler(uint32_t ulICCIAR)
{
	struct irq_handler *hdlr = NULL;
	unsigned int nr;
	extern uint64_t cpu_load_work_begin;

	/* CPU load: get a timestamp of the activity start */
	if (cpu_load_work_begin == 0)
		ARM_TIMER_GetCounterCount(ARM_TIMER_VIRTUAL, &cpu_load_work_begin);

	/*
	 * TODO: For now interrupt nesting is disabled,
	 * IRQ stays disabled until the handler returns
	 */

	nr = ulICCIAR & NR_IRQS;

	if (nr < NR_IRQS)
		hdlr = &handler[nr];

	if (hdlr && hdlr->func)
		hdlr->func(hdlr->data);
}
