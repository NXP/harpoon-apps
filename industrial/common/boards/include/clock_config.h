/*
 * Copyright 2022-2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _CLOCK_CONFIG_H_
#define _CLOCK_CONFIG_H_

#include <stdint.h>

void board_clock_setup(void);
int board_clock_get_flexcan_rate(uint32_t *rate);

#endif /* _CLOCK_CONFIG_H_ */
