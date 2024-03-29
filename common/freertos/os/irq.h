/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FREERTOS_IRQ_H_
#define _FREERTOS_IRQ_H_

#include <irq.h>
#include "fsl_common.h"

static inline int os_irq_register(unsigned int irq, void (*func)(void *data),
		void *data, unsigned int prio)
{
	return irq_register(irq, func, data, prio);
}

static inline int os_irq_unregister(unsigned int irq)
{
	return irq_unregister(irq);
}

static inline void os_irq_enable(unsigned int irq)
{
	EnableIRQ(irq);
}

static inline void os_irq_disable(unsigned int irq)
{
	DisableIRQ(irq);
}

#include "os/irq.h" /* include os priority abstractions from common/os/irq.h */

#endif /* #ifndef _FREERTOS_IRQ_H_ */
