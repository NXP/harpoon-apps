/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_MMU_H_
#define _COMMON_MMU_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/mmu.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/mmu.h"
#endif

int os_mmu_map(const char *name, uint8_t **virt, uintptr_t phys, size_t size, uint32_t attrs);

#endif /* _COMMON_MMU_H_ */
