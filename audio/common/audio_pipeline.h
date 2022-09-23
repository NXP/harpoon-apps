/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_PIPELINE_H_
#define _AUDIO_PIPELINE_H_

#include "audio_element.h"
#include "audio_buffer.h"

#if (CONFIG_GENAVB_ENABLE == 1)
#include "genavb/genavb.h"
#endif

#define AUDIO_PIPELINE_MAX_STAGES	4
#define AUDIO_PIPELINE_MAX_ELEMENTS	16
#define AUDIO_PIPELINE_MAX_BUFFERS	256

/* Configuration */
struct audio_pipeline_stage_config {
	unsigned int elements;
	struct audio_element_config element[AUDIO_PIPELINE_MAX_ELEMENTS];
};

struct audio_pipeline_config {
	char *name;

	unsigned int period;

	unsigned int sample_rate;

	unsigned int stages;
	struct audio_pipeline_stage_config stage[AUDIO_PIPELINE_MAX_STAGES];

	unsigned int buffers;
	struct audio_buffer_config buffer[AUDIO_PIPELINE_MAX_BUFFERS];

	unsigned int buffer_storage;
	struct audio_buffer_storage_config storage[AUDIO_PIPELINE_MAX_BUFFERS];
};

/* Run Time */
/* pipeline memory layout
 * stage[0]
 * stage[1]
 * ...
 * stage[n]
 * element[0]
 * element[1]
 * ...
 * element[m]
 * element[0]
 * element[1]
 * ...
 * element[l]
 * buffer[0]
 * buffer[1]
 * ...
 * buffer[k]
 * buffer storage
 */
struct audio_pipeline_stage {
	unsigned int elements;
	struct audio_element *element;
};

struct audio_pipeline {
	unsigned int stages;

	struct audio_pipeline_stage *stage;

	struct audio_buffer *buffer;
};

int audio_pipeline_ctrl(struct hrpn_cmd_audio_pipeline *cmd, unsigned int len, struct mailbox *m);
struct audio_pipeline *audio_pipeline_init(struct audio_pipeline_config *config);
int audio_pipeline_run(struct audio_pipeline *pipeline);
void audio_pipeline_exit(struct audio_pipeline *pipeline);
void audio_pipeline_reset(struct audio_pipeline *pipeline);
void audio_pipeline_dump(struct audio_pipeline *pipeline);
void audio_pipeline_stats(struct audio_pipeline *pipeline);

#endif /* _AUDIO_PIPELINE_H_ */
