/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "fsl_enet.h"
#include "fsl_phyar8031.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BOARD_PHY0_ADDRESS      (0x00U)           /* Phy address of enet port 0. */
#define BOARD_PHY0_OPS          &phyar8031_ops    /* PHY operations. */

#define BOARD_NET_PORT0_MII_MODE    kENET_RgmiiMode

extern phy_ar8031_resource_t phy_resource;

#endif /* _APP_BOARD_H_ */
