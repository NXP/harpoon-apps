/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define RT_LATENCY_TC 3

#define COUNTER_PERIOD_US_VAL (20000)

/*
 * Symbol definitions:
 *
 * WITH_CPU_LOAD:     Add CPU load in the lower priority task
 * WITH_CPU_LOAD_SEM: Add Semaphore load in CPU load thread
 */

#if (RT_LATENCY_TC == 2)
#define WITH_CPU_LOAD
#endif

#if (RT_LATENCY_TC == 4)
#define WITH_CPU_LOAD
#define WITH_CPU_LOAD_SEM
#endif

#if (RT_LATENCY_TC == 5)
#define WITH_CPU_LOAD
/* + Linux load */
#endif

#if (RT_LATENCY_TC == 6)
#define WITH_CPU_LOAD
#define WITH_INVD_CACHE
#endif

#if (RT_LATENCY_TC == 7)
#define WITH_CPU_LOAD
#define WITH_INVD_CACHE
/* with code in OCRAM (outer cacheable) */
/* + Linux load */
#endif
