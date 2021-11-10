/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define RT_LATENCY_TC 2

#define COUNTER_PERIOD_US_VAL (20000)

#if (RT_LATENCY_TC == 6)
#define WITH_INVD_CACHE
#endif

#if (RT_LATENCY_TC == 7)
#define WITH_INVD_CACHE
#endif

