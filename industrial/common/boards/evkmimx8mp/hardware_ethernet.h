/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HARDWARE_ETHERNET_H_
#define _HARDWARE_ETHERNET_H_

#include "fsl_enet_qos_mdio.h"

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode);

void hardware_ethernet_init(void);

#endif
