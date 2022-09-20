/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_MMU_H_
#define _ZEPHYR_MMU_H_

#include <zephyr/kernel.h>
#include <zephyr/arch/arm64/arm_mmu.h>
#include <zephyr/sys/mem_manage.h>

/* Memory attribute definitions */
/*
 * Caching mode definitions. These are mutually exclusive.
 */

/* Device memory with nGnRnE */
#define OS_MEM_DEVICE_nGnRnE	K_MEM_ARM_DEVICE_nGnRnE

/* Device memory with nGnRE */
#define OS_MEM_DEVICE_nGnRE	K_MEM_ARM_DEVICE_nGnRE

/* Device memory with GRE */
#define OS_MEM_DEVICE_GRE	K_MEM_ARM_DEVICE_GRE

/* No caching. */
#define OS_MEM_CACHE_NONE	K_MEM_CACHE_NONE

/* Write-through caching. */
#define OS_MEM_CACHE_WT		K_MEM_CACHE_WT

/* Full write-back caching. */
#define OS_MEM_CACHE_WB		K_MEM_CACHE_WB

/** Reserved bits for cache modes in k_map() flags argument */
#define OS_MEM_CACHE_MASK	K_MEM_CACHE_MASK

/*
 * Region permission attributes.
 */

/* Region will have read/write access */
#define OS_MEM_PERM_RW		K_MEM_PERM_RW

/* Region will be executable */
#define OS_MEM_PERM_EXEC	K_MEM_PERM_EXEC

/* Region will be accessible to user mode */
#define OS_MEM_PERM_USER	K_MEM_PERM_USER

static inline int os_mmu_map(const char *name, uint8_t **virt, uintptr_t phys, size_t size, uint32_t attrs)
{
	z_phys_map(virt, phys, size, attrs);

	return 0;
}

static inline int os_mmu_unmap(uintptr_t virt, size_t size)
{
	z_phys_unmap((uint8_t *)virt, size);

	return 0;
}

#endif /* #ifndef _ZEPHYR_MMU_H_ */
