/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/stdlib.h"
#include "os/stdio.h"

#include "audio_pipeline.h"

static int audio_pipeline_config_check(struct audio_pipeline_config *config)
{
	/* Sanity check pipeline configuration */

	/* Errors */
	/* Check all buffers point to existing memory */
	/* Check all buffers are referenced by a single input and single output */
	/* Check that all elements point to existing inputs/outputs */
	/* Check that no element input is connected to an ouput of a later stage */

	/* Warnings */
	/* Check all buffers are referenced by one input and one output */
	/* Check all buffer_storage is referenced by a buffer */

	/* Check the configuration of all elements */

	return 0;
}

static unsigned int audio_buffer_storage_size(struct audio_pipeline_config *config)
{
	unsigned int size = 0;
	int i;

	for (i = 0; i < config->buffer_storage; i++) {
		if (!config->storage[i].base)
			size += config->storage[i].periods * config->period;
	}

	return size;
}

static unsigned int audio_buffer_storage_off(struct audio_pipeline_config *config, unsigned int id)
{
	unsigned int offset = 0;
	int i;

	for (i = 0; i < id; i++) {
		if (!config->storage[i].base)
			offset += config->storage[i].periods * config->period;
	}

	return offset;
}

static unsigned int audio_buffer_size(struct audio_pipeline_config *config)
{
	return config->buffers * sizeof(struct audio_buffer);
}

static unsigned int audio_element_data_size_total(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	unsigned int size = 0;
	int i, j;

	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			size += audio_element_data_size(element_config);
		}
	}

	return size;
}

static unsigned int audio_element_size(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	unsigned int size = 0;
	int i;

	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];
		size += stage_config->elements * sizeof(struct audio_element);
	}

	return size;
}

static unsigned int audio_stage_size(struct audio_pipeline_config *config)
{
	return config->stages * sizeof(struct audio_pipeline_stage);
}

static struct audio_pipeline *audio_pipeline_alloc(struct audio_pipeline_config *config)
{
	struct audio_pipeline *pipeline;
	unsigned int size;

	size = sizeof(struct audio_pipeline);
	size += audio_stage_size(config);
	size += audio_element_size(config);
	size += audio_element_data_size_total(config);
	size += audio_buffer_size(config);
	size += audio_buffer_storage_size(config);

	pipeline = os_malloc(size);
	if (!pipeline)
		goto err;

	memset(pipeline, 0, size);

	return pipeline;

err:
	return NULL;
}

static void audio_pipeline_free(struct audio_pipeline *pipeline)
{
	os_free(pipeline);
}

static void audio_pipeline_buffer_init(struct audio_pipeline *pipeline, struct audio_pipeline_config *config)
{
	uint8_t *stage_base, *element_base, *element_data_base, *buffer_base, *buffer_storage_base;
	unsigned int storage_id;
	int32_t *base;
	unsigned int size;
	int i;

	stage_base = ((uint8_t *)pipeline + sizeof(struct audio_pipeline));
	element_base = ((uint8_t *)stage_base + audio_stage_size(config));
	element_data_base = ((uint8_t *)element_base + audio_element_size(config));

	buffer_base = ((uint8_t *)element_data_base + audio_element_data_size_total(config));
	buffer_storage_base = ((uint8_t *)buffer_base + audio_buffer_size(config));

	pipeline->buffer = (struct audio_buffer *)buffer_base;

	for (i = 0; i < config->buffers; i++) {
		storage_id = config->buffer[i].storage;

		base = (int32_t *)buffer_storage_base + audio_buffer_storage_off(config, storage_id);
		size = config->storage[storage_id].periods * config->period;

		audio_buf_init(&pipeline->buffer[i], base, size);
	}
}

static void audio_stage_init(struct audio_pipeline *pipeline, struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	uint8_t *stage_base, *element_base, *element_data_base;
	int i, j;

	stage_base = ((uint8_t *)pipeline + sizeof(struct audio_pipeline));
	element_base = ((uint8_t *)stage_base + audio_stage_size(config));
	element_data_base = ((uint8_t *)element_base + audio_element_size(config));

	pipeline->stages = config->stages;
	pipeline->stage = (struct audio_pipeline_stage *)stage_base;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];
		stage_config = &config->stage[i];

		stage->elements = stage_config->elements;
		stage->element = (struct audio_element *)element_base;
		element_base += stage->elements * sizeof(struct audio_element);

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];
			element_config = &stage_config->element[j];

			element->data = element_data_base;

			element_data_base += audio_element_data_size(element_config);
		}
	}
}

static struct audio_pipeline *audio_pipeline_create(struct audio_pipeline_config *config)
{
	struct audio_pipeline *pipeline;

	os_printf("%s: enter\n\r", __func__);

	if (audio_pipeline_config_check(config) < 0)
		goto err;

	pipeline = audio_pipeline_alloc(config);
	if (!pipeline)
		goto err;

	audio_stage_init(pipeline, config);

	audio_pipeline_buffer_init(pipeline, config);

	os_printf("%s: done\n\r", __func__);

	return pipeline;

err:
	return NULL;
}

static void audio_pipeline_set_config(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	int i, j;

	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			/* inherit default parameters if not overriden */
			if (!element_config->sample_rate)
				element_config->sample_rate = config->sample_rate;

			if (!element_config->period)
				element_config->period = config->period;
		}
	}
}

struct audio_pipeline *audio_pipeline_init(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	struct audio_pipeline *pipeline;
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	os_printf("%s: enter\n\r", __func__);

	audio_pipeline_set_config(config);

	pipeline = audio_pipeline_create(config);
	if (!pipeline)
		goto err_alloc;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];
		stage_config = &config->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];
			element_config = &stage_config->element[j];

			if (audio_element_init(element, element_config, pipeline->buffer) < 0)
				goto err_init;
		}
	}

	os_printf("%s: done\n\r", __func__);

	return pipeline;

err_init:
	/* call exit for all already initialized elements */
	audio_pipeline_free(pipeline);

err_alloc:
	return NULL;
}

int audio_pipeline_run(struct audio_pipeline *pipeline)
{
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];

			if (audio_element_run(element))
				goto err;
		}
	}

	return 0;

err:
	return -1;
}

void audio_pipeline_exit(struct audio_pipeline *pipeline)
{
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];

			audio_element_exit(element);
		}
	}

	audio_pipeline_free(pipeline);
}

void audio_pipeline_dump(struct audio_pipeline *pipeline)
{
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];

			audio_element_dump(element);
		}
	}
}
