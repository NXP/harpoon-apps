/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES		\
	MMU_REGION_FLAT_ENTRY("AUDIOMIX",				\
			      AUDIOMIX_BASE, KB(64),			\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("IOMUXC",					\
			      IOMUXC_BASE, KB(64),			\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("GPIO5",					\
			      GPIO5_BASE, KB(64),			\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("GPC",					\
			      GPC_BASE, KB(64),				\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI3",					\
			      I2S3_BASE, KB(64),			\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("I2C3",					\
			      I2C3_BASE, KB(64),			\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\
	MMU_REGION_FLAT_ENTRY("SAI5",					\
			      I2S5_BASE, KB(64),			\
			      MT_DEVICE_nGnRnE | MT_P_RW_U_RW | MT_NS),	\
									\

#endif /* _APP_MMU_H_ */
