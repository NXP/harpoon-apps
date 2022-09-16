/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_avtp_source.h"
#include "audio_element.h"
#include "audio_format.h"
#include "hlog.h"

/*
 * AVTP source: AVB audio stream listener
 */

struct avtp_source_element {
	struct audio_buffer *out;
	bool started;
};

static int avtp_source_element_run(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;
	audio_sample_t val = AUDIO_SAMPLE_SILENCE;
	int i;

	/* Fill output buffer with silence */
	for (i = 0; i < element->period; i++)
		__audio_buf_write(avtp->out, i, &val, 1);

	audio_buf_write_update(avtp->out, element->period);

	avtp->started = true;

	return 0;
}

static void avtp_source_element_reset(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;

	audio_buf_reset(avtp->out);

	avtp->started = false;
}

static void avtp_source_element_exit(struct audio_element *element)
{
}

static void avtp_source_element_dump(struct audio_element *element)
{
	struct avtp_source_element *avtp = element->data;

	log_info("avtp source(%p/%p)\n", avtp, element);
	log_info("  running: %u\n", avtp->started);

	audio_buf_dump(avtp->out);
}

int avtp_source_element_check_config(struct audio_element_config *config)
{
	if (config->inputs) {
		log_err("avtp source: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (config->outputs != 1) {
		log_err("avtp source: invalid outputs: %u\n", config->outputs);
		goto err;
	}

	return 0;

err:
	return -1;
}

unsigned int avtp_source_element_size(struct audio_element_config *config)
{
	return sizeof(struct avtp_source_element);
}

int avtp_source_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct avtp_source_element *avtp = element->data;

	element->run = avtp_source_element_run;
	element->reset = avtp_source_element_reset;
	element->exit = avtp_source_element_exit;
	element->dump = avtp_source_element_dump;

	avtp->started = false;
	avtp->out = &buffer[config->output[0]];

	avtp_source_element_dump(element);

	return 0;
}
