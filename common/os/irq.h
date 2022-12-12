/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_IRQ_H_
#define _COMMON_IRQ_H_

/* MAX priority is 0 in Zephyr, but defined by configMAX_API_CALL_INTERRUPT_PRIORITY
 * in FreeRTOS; let's take the most restrictive value here */
#define OS_IRQ_PRIO_MAX     9
#define OS_IRQ_PRIO_MIN     (15 - 1)
#define OS_IRQ_PRIO_DEFAULT ((OS_IRQ_PRIO_MIN + OS_IRQ_PRIO_MAX) / 2U)

#if defined(OS_ZEPHYR)
  #include "zephyr/os/irq.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/irq.h"
#endif

int os_irq_register(unsigned int irq, void (*func)(void *data),
		void *data, unsigned int prio);
int os_irq_unregister(unsigned int irq);
void os_irq_enable(unsigned int irq);
void os_irq_disable(unsigned int irq);

#endif /* #ifndef _COMMON_IRQ_H_ */
