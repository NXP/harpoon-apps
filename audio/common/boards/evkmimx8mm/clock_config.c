/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "sai_clock_config.h"

void BOARD_InitClocks(void)
{
	sai_clock_setup();
}
