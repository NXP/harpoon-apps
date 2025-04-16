/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>
#include "app_mmu.h"

void BOARD_InitMemory(void)
{
	mm_reg_t virt_mem;

	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(flexcan2)), DT_REG_SIZE(DT_NODELABEL(flexcan2)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(msgintr2)), DT_REG_SIZE(DT_NODELABEL(msgintr2)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(netc)), DT_REG_SIZE(DT_NODELABEL(netc)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(enetc_pf0)), DT_REG_SIZE(DT_NODELABEL(enetc_pf0)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(enetc_psi0)), DT_REG_SIZE(DT_NODELABEL(enetc_psi0)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(netc_timer)), DT_REG_SIZE(DT_NODELABEL(netc_timer)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(timer_pci)), DT_REG_SIZE(DT_NODELABEL(timer_pci)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(netc_timer_msix_table)), DT_REG_SIZE(DT_NODELABEL(netc_timer_msix_table)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(tpm4)), DT_REG_SIZE(DT_NODELABEL(tpm4)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
}
