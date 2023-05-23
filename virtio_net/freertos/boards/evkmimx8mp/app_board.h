/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_BOARD_H_
#define _APP_BOARD_H_

#include "fsl_enet.h"
#include "fsl_phyrtl8211f.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BOARD_PHY0_ADDRESS      (0x01U)           /* Phy address of enet port 0. */
#define BOARD_PHY0_OPS          &phyrtl8211f_ops    /* PHY operations. */

#define BOARD_NET_PORT0_MII_MODE    kENET_RgmiiMode

#define BOARD_ENET_CLOCK_FREQ       (CLOCK_GetFreq(kCLOCK_EnetIpgClk))

extern phy_rtl8211f_resource_t phy_resource;

#endif /* _APP_BOARD_H_ */
