/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _SAI_CONFIG_H_
#define _SAI_CONFIG_H_

struct sai_active_config {
	void *sai_base;
	uint32_t masterSlave;
	uint32_t msel;
	uint32_t audio_pll;
	uint32_t audio_pll_mul;
	uint32_t audio_pll_div;
	uint32_t slot_count; /* Number of words in audio frame: channels count */
	sai_word_width_t slot_size; /* Word size in bits */
	sai_sync_mode_t rx_sync_mode;
	sai_sync_mode_t tx_sync_mode;
	enum codec_id cid;
};

extern struct sai_active_config sai_active_list[];
extern int32_t sai_active_list_nelems;
void sai_set_audio_hat_codec(bool use_audio_hat, unsigned int rate);

#endif /* _SAI_CONFIG_H_ */
