/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HARDWARE_ETHERNET_H_
#define _HARDWARE_ETHERNET_H_

#include "fsl_enet.h"
#include "fsl_phyar8031.h"

extern phy_ar8031_resource_t phy_resource;

void hardware_ethernet_init(void);

#endif
