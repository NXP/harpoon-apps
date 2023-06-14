/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#define APP_MMU_ENTRIES							\
	MMU_REGION_FLAT_ENTRY("ENET",					\
			      ENET_BASE, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("VIRTIO",					\
			      0xfc700000, KB(4),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("GEN_MBOX",				\
			      0xfe000000, KB(4),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("MEMORY",					\
			      0x80000000, MB(0x200),			\
			      MT_NORMAL_NC | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("VIRTIO_DMA_BUF",				\
			      0xf8700000, MB(0x40),			\
			      MT_NORMAL_NC | MT_P_RW_U_RW | MT_NS),

#endif /* _APP_MMU_H_ */
