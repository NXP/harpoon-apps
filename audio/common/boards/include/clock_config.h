/*
 * Copyright 2022, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _CLOCK_CONFIG_H_
#define _CLOCK_CONFIG_H_

#include <stdint.h>

uint32_t BOARD_GetAudioPLLFreq(int pll_id);
void BOARD_InitClocks(void);

#endif /* _CLOCK_CONFIG_H_ */
