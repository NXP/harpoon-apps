/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_VIRTIO_H_
#define _APP_VIRTIO_H_

#include <os/irq.h>

#define CONFIG_CPU_STATS	1

#define ENET_IRQ_PRIO			(OS_IRQ_PRIO_MIN - 1)

#define VIRTIO_NET_MEM_BASE		0xfc700000

#define ENET_PORT_BASE			ENET
#define ENET_PORT_IRQ			ENET_IRQn

extern void ENET_DriverIRQHandler(void);
static inline void enet_hal_irq_handler(void)
{
	ENET_DriverIRQHandler();
}

#endif /* _APP_VIRTIO_H_ */
