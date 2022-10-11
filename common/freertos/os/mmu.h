/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_MMU_H_
#define _FREERTOS_MMU_H_

#include "os/stdint.h"
#include "os/stddef.h"
#include "mmu_armv8a.h"

/* Memory attribute definitions */
/*
 * Caching mode definitions. These are mutually exclusive.
 */

/* Device memory with nGnRnE */
#define OS_MEM_DEVICE_nGnRnE	5

/* Device memory with nGnRE */
#define OS_MEM_DEVICE_nGnRE	4

/* Device memory with GRE */
#define OS_MEM_DEVICE_GRE	3

/* No caching. */
#define OS_MEM_CACHE_NONE	2

/* Write-through caching. */
#define OS_MEM_CACHE_WT		1

/* Full write-back caching. */
#define OS_MEM_CACHE_WB		0

/** Reserved bits for cache modes in k_map() flags argument */
#define OS_MEM_CACHE_MASK	((1 << 3) - 1)

/*
 * Region permission attributes.
 */

/* Region will have read/write access */
#define OS_MEM_PERM_RW		(1 << 3)

/* Region will be executable */
#define OS_MEM_PERM_EXEC	(1 << 4)

/* Region will be accessible to user mode */
#define OS_MEM_PERM_USER	(1 << 5)


static inline int os_mmu_map(const char *name, uint8_t **virt, uintptr_t phys, size_t size, uint32_t attrs)
{
	uint32_t flags = 0;
	int rc;

	switch (attrs & OS_MEM_CACHE_MASK) {
	case OS_MEM_CACHE_NONE:
		flags |= MT_NORMAL_NC;
		break;
	case OS_MEM_DEVICE_nGnRnE:
		flags |= MT_DEVICE_nGnRnE;
		break;
	case OS_MEM_DEVICE_nGnRE:
		flags |= MT_DEVICE_nGnRE;
		break;
	case OS_MEM_DEVICE_GRE:
		flags |= MT_DEVICE_GRE;
		break;
	case OS_MEM_CACHE_WT:
		flags |= MT_NORMAL_WT;
		break;
	case OS_MEM_CACHE_WB:
		flags |= MT_NORMAL;
		break;
	default:
		return -1;
	}

	if ((attrs & OS_MEM_PERM_RW) != 0U) {
		flags |= MT_RW;
	}

	if ((attrs & OS_MEM_PERM_EXEC) == 0U) {
		flags |= MT_P_EXECUTE_NEVER;
		flags |= MT_U_EXECUTE_NEVER;
	}

	/* Set is to be NS access by default */
	flags |= MT_NS;
	/* Mirror RO/RW permissions to EL0 */
	flags |= MT_RW_AP_ELx;

	/* Use phys vs virt 1:1 mapping */
	rc = ARM_MMU_AddMap(name, phys, phys, size, flags);
	if (rc < 0)
		goto out;

	*virt = (uint8_t *)phys;

out:
	return rc;
}

static inline int os_mmu_unmap(uintptr_t virt, size_t size)
{
	return 0;
}

#endif /* #ifndef _FREERTOS_MMU_H_ */
