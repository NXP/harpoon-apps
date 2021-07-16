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

struct irq_handler {
	void (*func)(void *data);
	void *data;
};

static struct irq_handler handler[NR_IRQS];

void irq_handler(int nr)
{
	struct irq_handler *hdlr = &handler[nr];

	if (hdlr)
		hdlr->func(hdlr->data);
}


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
