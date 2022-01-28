/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_DTMF_H_
#define _AUDIO_ELEMENT_DTMF_H_

#include "audio_buffer.h"

struct dtmf_element_config {
	unsigned int us;
	unsigned int pause_us;
	unsigned int sequence_pause_us;

	double amplitude;

	char *sequence;
};

struct audio_element_config;
struct audio_element;

unsigned int dtmf_element_size(struct audio_element_config *config);
int dtmf_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_DTMF_H_ */
