/*
 *  * Copyright 2022 NXP
 *   * All rights reserved.
 *    *
 *     * SPDX-License-Identifier: BSD-3-Clause
 *      */

#ifndef _ENET_QOS_DRV_H_
#define _ENET_QOS_DRV_H_

#include "fsl_enet_qos_mdio.h"
#include "fsl_phyrtl8211f.h"

#define EXAMPLE_ENET_QOS_BASE   ENET_QOS
#define EXAMPLE_PHY_ADDR        0x01U
#define EXAMPLE_ENET_QOS_OPS    enet_qos_ops
#define EXAMPLE_PHY             phyrtl8211f_ops
#define EXAMPLE_ENET_QOS_IRQ    ENET_QOS_IRQn

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode);

void enet_qos_hardware_init(void);

#endif
