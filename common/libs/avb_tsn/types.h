/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#define NSECS_PER_SEC  (1000000000ULL)
#define NSECS_PER_MSEC (1000000)
#define NSECS_PER_USEC (1000)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define offset_of(type, member)           ((unsigned long)&(((type *)0)->member))
#define container_of(entry, type, member) ((type *)((unsigned char *)(entry)-offset_of(type, member)))

#endif /* _TYPES_H_*/
