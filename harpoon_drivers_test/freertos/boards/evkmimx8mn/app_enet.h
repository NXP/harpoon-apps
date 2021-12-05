/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_ENET_H_
#define _APP_ENET_H_

#include "fsl_enet_mdio.h"
#include "fsl_phyar8031.h"

#define ENET_PHY_ADDRESS                0x0U
#define ENET_PHY_MIIMODE                kENET_RgmiiMode
/* MDIO operations. */
#define ENET_MDIO_OPS                   enet_ops
/* PHY operations. */
#define ENET_PHY_OPS                    phyar8031_ops

#define NONCACHEABLE(var, alignbytes)   AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes)

#endif /* _APP_ENET_H_ */
