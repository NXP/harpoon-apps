/*
* Copyright 2023 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "hardware_flexcan.h"

void hardware_flexcan_init(void)
{
	/* GPIO Initialization done externally by Linux Harpoon script */

	/*
	* Set CAN_STBY C0 PIN of the ADP5585 as an output in an active mode
	* Set EXP_SEL R4 PIN of the ADP5585 as an output in an active mode
	*/
	return;
}
