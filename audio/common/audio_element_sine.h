/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_SINE_H_
#define _AUDIO_ELEMENT_SINE_H_

#include "audio_buffer.h"

struct sine_element_config {
	double freq;
	double amplitude;
};

struct audio_element_config;
struct audio_element;

int sine_element_check_config(struct audio_element_config *config);
unsigned int sine_element_size(struct audio_element_config *config);
int sine_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_SINE_H_ */
