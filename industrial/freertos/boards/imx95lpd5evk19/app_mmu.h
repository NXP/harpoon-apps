/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES		\
	MMU_REGION_FLAT_ENTRY("CAN2",					\
			      CAN2_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("MSGINTR2",                               \
			      MSGINTR2_BASE, KB(64),                    \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("ENETC0_PCI_HDR_TYPE0",                   \
			      ENETC0_PCI_HDR_TYPE0_BASE, KB(64),        \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("TMR0_PCI_HDR_TYPE0_BASE",                \
			      TMR0_PCI_HDR_TYPE0_BASE, KB(64),          \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("ENETC0_PSI",                             \
			      ENETC0_PSI_BASE, KB(64),                  \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("ENETC0_PF",                              \
			      ENETC0_BASE_BASE, KB(64),                 \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("TMR0",                                   \
			      TMR0_BASE_BASE, KB(64),                   \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \
	MMU_REGION_FLAT_ENTRY("NETC_TIMER_MSIX_TABLE",                  \
			      FSL_FEATURE_NETC_MSIX_TABLE_BASE, KB(64), \
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),  \

#endif /* _APP_MMU_H_ */
