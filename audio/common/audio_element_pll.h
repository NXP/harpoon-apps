/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_PLL_H_
#define _AUDIO_ELEMENT_PLL_H_

#include "audio_buffer.h"

#include "hrpn_ctrl_audio_pipeline.h"

struct pll_element_config {
	unsigned int src_sai_id;
	unsigned int dst_sai_id;
	unsigned int pll_id;
};

struct audio_element_config;
struct audio_element;

int pll_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_pll *cmd, unsigned int len, void *ctrl_handle);
int pll_element_check_config(struct audio_element_config *config);
unsigned int pll_element_size(struct audio_element_config *config);
int pll_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_PLL_H_ */
