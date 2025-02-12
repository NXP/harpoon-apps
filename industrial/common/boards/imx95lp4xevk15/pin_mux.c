/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pinctrl.h"

void BOARD_InitPins(void)
{
	pin_mux_flexcan();
#if defined(CONFIG_USE_GENAVB) && (CONFIG_USE_GENAVB == 1)
	pin_mux_enetc();
#endif
}
