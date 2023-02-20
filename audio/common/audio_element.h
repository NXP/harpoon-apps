/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_ELEMENT_H_
#define _AUDIO_ELEMENT_H_

#if (CONFIG_GENAVB_ENABLE == 1)
#include "audio_element_avtp_sink.h"
#include "audio_element_avtp_source.h"
#endif
#include "audio_element_dtmf.h"
#include "audio_element_pll.h"
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
	AUDIO_ELEMENT_PLL,
#if (CONFIG_GENAVB_ENABLE == 1)
	AUDIO_ELEMENT_AVTP_SOURCE, /* AVB audio stream listener */
	AUDIO_ELEMENT_AVTP_SINK,   /* AVB audio stream talker */
#endif
	AUDIO_ELEMENT_MAX,
};

extern const char *element_name[AUDIO_ELEMENT_MAX];

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
		struct pll_element_config pll;
		struct routing_element_config routing;
		struct sai_sink_element_config sai_sink;
		struct sai_source_element_config sai_source;
		struct sine_element_config sine;
#if (CONFIG_GENAVB_ENABLE == 1)
		struct avtp_source_element_config avtp_source;
		struct avtp_sink_element_config avtp_sink;
#endif
	} u;
};

/* Run Time */
struct audio_element {
	void *data;

	unsigned int type;
	unsigned int sample_rate;
	unsigned int period;
	unsigned int element_id;

	int (*run)(struct audio_element *element);
	void(*reset)(struct audio_element *element);
	void(*exit)(struct audio_element *element);
	void(*dump)(struct audio_element *element);
	void(*stats)(struct audio_element *element);
};

struct mailbox;

int audio_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element *cmd, unsigned int len, struct mailbox *m);
void audio_element_exit(struct audio_element *element);
void audio_element_dump(struct audio_element *element);
void audio_element_stats(struct audio_element *element);
int audio_element_check_config(struct audio_element_config *config);
unsigned int audio_element_data_size(struct audio_element_config *config);
int audio_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer);

static inline int audio_element_run(struct audio_element *element)
{
	return element->run(element);
}

static inline void audio_element_reset(struct audio_element *element)
{
	element->reset(element);
}

#endif /* _AUDIO_ELEMENT_H_ */
