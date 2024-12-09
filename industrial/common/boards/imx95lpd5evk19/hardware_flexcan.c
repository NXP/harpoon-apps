/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware_flexcan.h"

void hardware_flexcan_init(void)
{
	/* GPIO Initialization done externally by Linux Harpoon script */

	/*
	 * Set CAN2_EN PIN of the PCAL6524 as an output in an active-high mode
	 * Set CAN2_nSTBY PIN of the PCAL6524 as an output in an active-high mode
	 */
	return;
}
