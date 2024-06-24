/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_ENET_H_
#define _APP_ENET_H_

#include "fsl_enet.h"
#include "fsl_phyar8031.h"

#define ENET_PHY_ADDRESS                0x0U
#define ENET_PHY_MIIMODE                kENET_RgmiiMode
/* PHY operations. */
#define ENET_PHY_OPS                    phyar8031_ops

#define NONCACHEABLE(var, alignbytes)   AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes)

phy_ar8031_resource_t phy_resource;

#endif /* _APP_ENET_H_ */
