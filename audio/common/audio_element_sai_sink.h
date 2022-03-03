/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_SAI_SINK_H_
#define _AUDIO_ELEMENT_SAI_SINK_H_

#include "audio_buffer.h"

#define SAI_TX_MAX_INSTANCE		6
#define SAI_TX_MAX_ID			8
#define SAI_TX_INSTANCE_MAX_LINE	8
#define SAI_TX_INSTANCE_MAX_CHANNELS	8

/* Fixed mapping between input buffers and sai instances/lines/channels
 * e.g
 * input 0 -> sai0, line0, ch0
 * input 1 -> sai0, line0, ch1
 * input 2 -> sai0, line1, ch0
 * input 3 -> sai0, line2, ch0
 * input 4 -> sai1, line0, ch0
 * input 5 -> sai1, line1, ch0
 * input 6 -> sai2, line0, ch0
 */
struct sai_sink_element_config {
	unsigned int sai_n;			/* number of sai instances */

	struct sai_tx_config {
		unsigned int id;		/* sai instance */

		unsigned int line_n;		/* number of physical lines for the sai instance */

		struct sai_tx_line_config {
			unsigned int id;	/* line id for the sai instance */

			unsigned channel_n;	/* number of audio channels for the physical line */
		} line [SAI_TX_INSTANCE_MAX_LINE];
	} sai [SAI_TX_MAX_INSTANCE];
};

struct audio_element_config;
struct audio_element;

int sai_sink_element_check_config(struct audio_element_config *config);
unsigned int sai_sink_element_size(struct audio_element_config *config);
int sai_sink_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

#endif /* _AUDIO_ELEMENT_SAI_SINK_H_ */
