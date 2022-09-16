/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_AVTP_SOURCE_H_
#define _AUDIO_ELEMENT_AVTP_SOURCE_H_

#include "audio_buffer.h"

struct avtp_source_element_config {
};

struct audio_element_config;
struct audio_element;

int avtp_source_element_check_config(struct audio_element_config *config);
unsigned int avtp_source_element_size(struct audio_element_config *config);
int avtp_source_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_AVTP_SOURCE_H_ */
