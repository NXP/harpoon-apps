/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_AVTP_SOURCE_H_
#define _AUDIO_ELEMENT_AVTP_SOURCE_H_

#include "audio_buffer.h"
#include "genavb/control_clock_domain.h"
#include "hrpn_ctrl_audio_pipeline.h"

#define AVTP_RX_STREAM_N		2
#define AVTP_RX_CHANNEL_N		2

#define AUDIO_ELEMENT_AVTP_SOURCE_MAX	1

struct avtp_source_element_config {
	unsigned int stream_n;			/* number of streams */

	struct avtp_source_stream_config {
		unsigned int flags;
	} stream [AVTP_RX_STREAM_N];

	genavb_clock_domain_t clock_domain;
};

struct audio_element_config;
struct audio_element;

int avtp_source_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_avtp *cmd, unsigned int len, void *ctrl_handle);
int avtp_source_element_check_config(struct audio_element_config *config);
unsigned int avtp_source_element_size(struct audio_element_config *config);
int avtp_source_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_AVTP_SOURCE_H_ */
