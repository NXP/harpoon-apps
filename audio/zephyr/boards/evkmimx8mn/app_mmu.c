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

	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(i2c3)), DT_REG_SIZE(DT_NODELABEL(i2c3)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(sai3)), DT_REG_SIZE(DT_NODELABEL(sai3)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(sai5)), DT_REG_SIZE(DT_NODELABEL(sai5)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(enet)), DT_REG_SIZE(DT_NODELABEL(enet)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(gpt2)), DT_REG_SIZE(DT_NODELABEL(gpt2)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
}
