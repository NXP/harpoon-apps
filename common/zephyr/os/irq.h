/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ZEPHYR_IRQ_H_
#define _ZEPHYR_IRQ_H_

#include <zephyr/kernel.h>

static inline int os_irq_register(unsigned int irq, void (*func)(void *data),
		void *data, unsigned int prio)
{
	irq_connect_dynamic(irq, prio, (void (*)(const void *data))func,
			(const void *)data, 0);
	return 0;
}

static inline int os_irq_unregister(unsigned int irq)
{
	/* To be safe in case of not calling os_irq_disable() following this api */
	irq_disable(irq);

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
