/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pinctrl.h"

void BOARD_InitPins(void)
{
	pin_mux_flexcan();
}
