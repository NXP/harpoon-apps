
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

	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(gpc)), DT_REG_SIZE(DT_NODELABEL(gpc)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(i2c3)), DT_REG_SIZE(DT_NODELABEL(i2c3)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(sai3)), DT_REG_SIZE(DT_NODELABEL(sai3)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(sai5)), DT_REG_SIZE(DT_NODELABEL(sai5)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(audio_blk_ctrl)), DT_REG_SIZE(DT_NODELABEL(audio_blk_ctrl)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(gpio4)), DT_REG_SIZE(DT_NODELABEL(gpio4)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(gpio5)), DT_REG_SIZE(DT_NODELABEL(gpio5)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(iomuxc_gpr)), DT_REG_SIZE(DT_NODELABEL(iomuxc_gpr)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(enet_qos)), DT_REG_SIZE(DT_NODELABEL(enet_qos)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
	device_map(&virt_mem, DT_REG_ADDR(DT_NODELABEL(gpt1)), DT_REG_SIZE(DT_NODELABEL(gpt1)), K_MEM_DIRECT_MAP | K_MEM_CACHE_NONE);
}
