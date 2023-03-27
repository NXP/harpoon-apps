/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES		\
	MMU_REGION_FLAT_ENTRY("AUDIOMIX",				\
			      AUDIOMIX_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("ENET_QOS",				\
			      ENET_QOS_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("GPC",					\
			      GPC_BASE, KB(64),				\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("GPIO4",					\
			      GPIO4_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("I2C3",					\
			      I2C3_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("IOMUXC_GPR",				\
			      IOMUXC_GPR_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
									\
	MMU_REGION_FLAT_ENTRY("SAI1",					\
			      I2S1_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI2",					\
			      I2S2_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI3",					\
			      I2S3_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI5",					\
			      I2S5_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI6",					\
			      I2S6_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI7",					\
			      I2S7_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\

#endif /* _APP_MMU_H_ */
