/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware_ethernet.h"
#include "ethernet_setup.h"

void hardware_ethernet_init(void)
{
	/* On Linux:
	 * - Power up NETCMIX
	 * - Configure BLK_CTRL_NETCMIX: set CFG_LINK_MII_PROT as RGMII for Port 0
	 * - Configure IERB: EFAUXR, VFAUXR
	 * - Configure IERB: Set SoC link Phy Address
	 */
	netcmix_init();
}
