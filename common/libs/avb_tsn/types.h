/*
 * Copyright 2018, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stddef.h>     /* for offsetof() */

#define NSECS_PER_SEC  (1000000000ULL)
#define NSECS_PER_MSEC (1000000)
#define NSECS_PER_USEC (1000)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define container_of(entry, type, member) ((type *)((unsigned char *)(entry) - offsetof(type, member)))

#endif /* _TYPES_H_*/
