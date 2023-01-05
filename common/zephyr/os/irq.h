/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ZEPHYR_IRQ_H_
#define _ZEPHYR_IRQ_H_

#include <zephyr/kernel.h>

#define PRIORITY_SHIFT 4

static inline int os_irq_register(unsigned int irq, void (*func)(void *data),
		void *data, unsigned int prio)
{
	if (prio < OS_IRQ_PRIO_MAX || prio > OS_IRQ_PRIO_MIN)
		return -1;

	irq_connect_dynamic(irq, prio << PRIORITY_SHIFT, (void (*)(const void *data))func,
			(const void *)data, 0);

	return 0;
}

static inline int os_irq_unregister(unsigned int irq)
{
	irq_disable(irq);
	irq_connect_dynamic(irq, OS_IRQ_PRIO_DEFAULT << PRIORITY_SHIFT, NULL, NULL, 0);

	return 0;
}

static inline void os_irq_enable(unsigned int irq)
{
	irq_enable(irq);
}

static inline void os_irq_disable(unsigned int irq)
{
	irq_disable(irq);
}

#endif /* #ifndef _ZEPHYR_IRQ_H_ */
