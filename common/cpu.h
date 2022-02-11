/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_CPU_H_
#define _COMMON_CPU_H_

#include "os/stdio.h"

#ifdef OS_ZEPHYR /* TODO: Implement cache invalidation with OS-independant code */
#include <zephyr.h>
#include <kernel.h>
#include <cache.h>
#endif

static inline void os_invd_dcache_all()
{
#ifndef OS_ZEPHYR /* TODO: Implement cache invalidation with OS-independant code */
    static int warn_once = 0;

    if (!warn_once++)
        os_printf("WARNING:  TODO: Flush D-Cache\n\r ");
#else
    sys_cache_data_all(K_CACHE_INVD);
#endif
}

static inline void os_invd_icache_all()
{
    __asm volatile ("IC IALLUIS");
}

#endif /* #ifndef _COMMON_CPU_H_ */
