/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES		\
	MMU_REGION_FLAT_ENTRY("CAN2",				\
			      CAN2_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("IOMUXC1",				\
			      IOMUXC1_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \

#endif /* _APP_MMU_H_ */
