/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_VIRTIO_H_
#define _APP_VIRTIO_H_

#include <os/irq.h>

#define CONFIG_CPU_STATS	1

#define VIRTIO_GEN_SW_MBOX_BASE		(0xb8500000)
#define VIRTIO_GEN_SW_MBOX_IRQ		(109)
#define VIRTIO_GEN_SW_MBOX_REMOTE_IRQ	(108)
#define VIRTIO_GEN_SW_MBOX_IRQ_PRIO	(OS_IRQ_PRIO_MIN - 1)

#define ENET_IRQ_PRIO			(OS_IRQ_PRIO_MIN - 1)

#define VIRTIO_NET_MEM_BASE		0xb8400000

#endif /* _APP_VIRTIO_H_ */
