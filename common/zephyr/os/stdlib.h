/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_STDLIB_H_
#define _ZEPHYR_STDLIB_H_

#define os_malloc(x)	k_malloc(x)
#define os_free(x)	k_free(x)

#endif /* #ifndef _ZEPHYR_STDLIB_H_ */
