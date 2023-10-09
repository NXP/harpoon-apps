/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_INIT_H_
#define CLOCK_INIT_H_

#include "fsl_clock.h"

void BOARD_ClockSourceFreqInit();
void BOARD_LpuartClockSetup();

#endif /* CLOCK_INIT_H_ */