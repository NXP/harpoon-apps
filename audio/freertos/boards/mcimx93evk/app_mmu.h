/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES		\
	MMU_REGION_FLAT_ENTRY("ENET_QOS",				\
			      ENET_QOS_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("LPI2C1",					\
			      LPI2C1_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI1",					\
			      SAI1_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI2",					\
			      SAI2_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI3",					\
			      SAI3_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\

#endif /* _APP_MMU_H_ */
