/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _SAI_CONFIG_H_
#define _SAI_CONFIG_H_

struct sai_active_config {
	void *sai_base;
	uint32_t masterSlave;
	uint32_t msel;
	clock_pll_ctrl_t audio_pll;
	uint32_t audio_pll_mul;
	uint32_t audio_pll_div;
	sai_sync_mode_t rx_sync_mode;
	sai_sync_mode_t tx_sync_mode;
	enum codec_id cid;
};

extern const struct sai_active_config sai_active_list[];
extern const int32_t sai_active_list_nelems;

#endif /* _SAI_CONFIG_H_ */
