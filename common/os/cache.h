/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_CACHE_H_
#define _COMMON_CACHE_H_

#include "fsl_cache.h"
#include "os/stdio.h"

#if defined(OS_ZEPHYR)
#define NONCACHEABLE	__nocache
#elif defined(FSL_RTOS_FREE_RTOS)
#define NONCACHEABLE	__attribute__((section("NonCacheable")))
#endif

#define OS_CACHE_WB      BIT(0)
#define OS_CACHE_INVD    BIT(1)
#define OS_CACHE_WB_INVD (OS_CACHE_WB | OS_CACHE_INVD)

static inline int os_dcache_data_range(uintptr_t addr, size_t size, int op)
{
	int ret = 0;

	switch (op) {
	case OS_CACHE_WB_INVD:
		DCACHE_CleanInvalidateByRange(addr, size);
		break;
	case OS_CACHE_INVD:
		DCACHE_InvalidateByRange(addr, size);
		break;
	case OS_CACHE_WB:
		DCACHE_CleanByRange(addr, size);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static inline void os_dcache_invd_all()
{
	static int warn_once = 0;

	if (!warn_once++)
		os_printf("WARNING:  TODO: Flush D-Cache\n\r ");
}

static inline void os_icache_invd_all()
{
	__asm volatile ("IC IALLUIS");
}

#endif /* _COMMON_CACHE_H_ */
