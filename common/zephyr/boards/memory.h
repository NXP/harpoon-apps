/*
 * Copyright 2022 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#ifdef CONFIG_BOARD_MIMX8MP_EVK_A53

#define	HYPERVISOR_COMM_BASE	0x80000000
#define	PCI_MMIO_BASE		0xff000000

#elif defined (CONFIG_BOARD_MIMX8MN_EVK_A53)

#define	HYPERVISOR_COMM_BASE	0x80000000
#define	PCI_MMIO_BASE		0xff000000

#elif defined (CONFIG_BOARD_MIMX8MM_EVK_A53)

#define	HYPERVISOR_COMM_BASE	0x80000000
#define	PCI_MMIO_BASE		0xff000000

#endif

#endif /* _MEMORY_H_ */
