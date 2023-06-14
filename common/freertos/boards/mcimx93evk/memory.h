/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

/* Virtual address space */
#define HYPERVISOR_COMM_BASE	0x80000000
#define DDR_BASE		0xd0000000
#define PCI_CFG_BASE		0xfd700000
#define PCI_IVSHMEM_BASE	0xfd9f0000
#define PCI_MMIO_BASE		0xff000000

#endif /* _MEMORY_H_ */
