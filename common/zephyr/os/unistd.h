/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ZEPHYR_UNISTD_H_
#define _ZEPHYR_UNISTD_H_

#include <zephyr/kernel.h>

static inline int os_msleep(int32_t msec)
{
    return k_msleep(msec);
}

#endif /* #ifndef _ZEPHYR_UNISTD_H_ */
