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

	/* ANA_PLL already mapped by Zephyr early MMU mappings */
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(wakeupmix_gpr)), DT_REG_SIZE(DT_NODELABEL(wakeupmix_gpr)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(enet_qos)), DT_REG_SIZE(DT_NODELABEL(enet_qos)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	/* TPM2 used as a counter through the zephyr native driver API, no need to map here */
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(tpm4)), DT_REG_SIZE(DT_NODELABEL(tpm4)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(flexcan2)), DT_REG_SIZE(DT_NODELABEL(flexcan2)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
}
