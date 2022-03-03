/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_ROUTING_H_
#define _AUDIO_ELEMENT_ROUTING_H_

#include "audio_buffer.h"

#include "hrpn_ctrl_audio_pipeline.h"

struct routing_element_config {
};

struct audio_element_config;
struct audio_element;

struct mailbox;

void routing_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_routing *cmd, unsigned int len, struct mailbox *m);
int routing_element_check_config(struct audio_element_config *config);
unsigned int routing_element_size(struct audio_element_config *config);
int routing_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_ROUTING_H_ */
