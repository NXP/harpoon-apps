/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HARDWARE_ETHERNET_H_
#define _HARDWARE_ETHERNET_H_

#include "fsl_enet_qos.h"
#include "fsl_phyrtl8211f.h"
#include "app_board.h"

extern phy_rtl8211f_resource_t phy_resource;

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode);
void ENET_QOS_EnableClock(bool enable);

void hardware_ethernet_init(void);

#endif
