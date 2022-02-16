/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_H_
#define _AUDIO_ELEMENT_H_

#include "audio_element_dtmf.h"
#include "audio_element_routing.h"
#include "audio_element_sai_sink.h"
#include "audio_element_sai_source.h"
#include "audio_element_sine.h"

#include "hrpn_ctrl_audio_pipeline.h"

#define AUDIO_ELEMENT_MAX_INPUTS	64
#define AUDIO_ELEMENT_MAX_OUTPUTS	64

enum {
	AUDIO_ELEMENT_DTMF_SOURCE = 0,
	AUDIO_ELEMENT_ROUTING,
	AUDIO_ELEMENT_SAI_SINK,
	AUDIO_ELEMENT_SAI_SOURCE,
	AUDIO_ELEMENT_SINE_SOURCE,
};

/* Configuration */
struct audio_element_config {
	unsigned int type;

	unsigned int inputs;
	unsigned int input[AUDIO_ELEMENT_MAX_INPUTS]; /* indexes to buffer structures */

	unsigned int outputs;
	unsigned int output[AUDIO_ELEMENT_MAX_OUTPUTS]; /* indexes to buffer structures */

	unsigned int period;
	unsigned int sample_rate;

	union {
		struct dtmf_element_config dtmf;
		struct routing_element_config routing;
		struct sai_sink_element_config sai_sink;
		struct sai_source_element_config sai_source;
		struct sine_element_config sine;
	} u;
};

/* Run Time */
struct audio_element {
	void *data;

	unsigned int type;
	unsigned int sample_rate;
	unsigned int period;

	int (*run)(struct audio_element *element);
	void(*reset)(struct audio_element *element);
	void(*exit)(struct audio_element *element);
	void(*dump)(struct audio_element *element);
};

struct mailbox;

void audio_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element *cmd, unsigned int len, struct mailbox *m);
unsigned int audio_element_data_size(struct audio_element_config *config);
int audio_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);
void audio_element_exit(struct audio_element *element);
void audio_element_dump(struct audio_element *element);

static inline int audio_element_run(struct audio_element *element)
{
	return element->run(element);
}

static inline void audio_element_reset(struct audio_element *element)
{
	element->reset(element);
}
#endif /* _AUDIO_ELEMENT_H_ */
