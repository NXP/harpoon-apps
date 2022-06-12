/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES		\
									\
	MMU_REGION_FLAT_ENTRY("ENET_QOS",				\
			      ENET_QOS_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("FLEXCAN1",				\
			      FLEXCAN1_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("GPIO4",					\
			      GPIO4_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("GPIO5",					\
			      GPIO5_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("IOMUXC_GPR",				\
			      IOMUXC_GPR_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \

#endif /* _APP_MMU_H_ */
