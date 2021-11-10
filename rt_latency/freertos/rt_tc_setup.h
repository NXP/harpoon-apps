/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define RT_LATENCY_TC 2

#define COUNTER_PERIOD_US_VAL (20000)

/*
 * Symbol definitions:
 *
 * WITH_CPU_LOAD_SEM: Add Semaphore load in CPU load thread
 */

#if (RT_LATENCY_TC == 4)
#define WITH_CPU_LOAD_SEM
#endif

#if (RT_LATENCY_TC == 6)
#define WITH_INVD_CACHE
#endif

#if (RT_LATENCY_TC == 7)
#define WITH_INVD_CACHE
#endif

