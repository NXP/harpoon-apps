/*
* Copyright 2023 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "hardware_ethernet.h"
#include "fsl_enet_qos.h"
#include "fsl_phy.h"

phy_rtl8211f_resource_t phy_resource;

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
	BLK_CTRL_WAKEUPMIX1->GPR |= BLK_CTRL_WAKEUPMIX_GPR_MODE(miiMode);
}

void ENET_QOS_EnableClock(bool enable)
{
	 BLK_CTRL_WAKEUPMIX1->GPR =
		(BLK_CTRL_WAKEUPMIX1->GPR & (~BLK_CTRL_WAKEUPMIX_GPR_ENABLE_MASK)) | BLK_CTRL_WAKEUPMIX_GPR_ENABLE(enable);
}

void hardware_ethernet_init(void)
{
	return;
}
