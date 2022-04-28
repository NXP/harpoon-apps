/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ENET_QOS_DRV_H_
#define _ENET_QOS_DRV_H_

#include "fsl_enet_qos_mdio.h"

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode);

void enet_qos_hardware_init(void);

#endif
