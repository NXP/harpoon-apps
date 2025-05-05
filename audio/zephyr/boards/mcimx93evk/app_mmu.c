
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

	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(wakeupmix_gpr)), DT_REG_SIZE(DT_NODELABEL(wakeupmix_gpr)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(lpi2c1)), DT_REG_SIZE(DT_NODELABEL(lpi2c1)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(lpi2c4)), DT_REG_SIZE(DT_NODELABEL(lpi2c4)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(sai3)), DT_REG_SIZE(DT_NODELABEL(sai3)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(enet_qos)), DT_REG_SIZE(DT_NODELABEL(enet_qos)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(gpio2)), DT_REG_SIZE(DT_NODELABEL(gpio2)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(tpm2)), DT_REG_SIZE(DT_NODELABEL(tpm2)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
}
