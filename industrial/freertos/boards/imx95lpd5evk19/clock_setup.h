/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLOCK_SETUP_H_
#define _CLOCK_SETUP_H_

#include <stdint.h>

void BOARD_TpmClockSetup(void);
void clock_setup_flexcan(void);
int clock_get_flexcan_clock(uint32_t *rate);
void clock_setup_enetc(void);

#endif /* _CLOCK_SETUP_H_ */
