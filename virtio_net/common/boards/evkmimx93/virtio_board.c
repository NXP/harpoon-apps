/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_phyrtl8211f.h"

#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

phy_rtl8211f_resource_t phy_resource;

void virtio_board_init(void)
{
	/* Init board cpu and hardware. */
	BOARD_InitMemory();
	/* Enable GIC before register any interrupt handler*/
	GIC_Enable(1);
	board_pins_setup();
	board_clock_setup();
	BOARD_InitDebugConsole();
}
