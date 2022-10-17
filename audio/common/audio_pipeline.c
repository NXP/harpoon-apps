/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/stdlib.h"

#include "audio_pipeline.h"
#include "hrpn_ctrl.h"
#include "hlog.h"
#include "mailbox.h"

#define MAX_PIPELINES	4
#define STORAGE_DEFAULT_PERIODS 2

static struct audio_pipeline *pipeline_table[MAX_PIPELINES];

static int audio_pipeline_table_add(struct audio_pipeline *pipeline)
{
	int i;

	for (i = 0; i < MAX_PIPELINES; i++) {
		if (!pipeline_table[i]) {
			pipeline_table[i] = pipeline;
			goto done;
		}
	}

	return -1;

done:
	return 0;
}

static void audio_pipeline_table_del(struct audio_pipeline *pipeline)
{
	int i;

	for (i = 0; i < MAX_PIPELINES; i++)
		if (pipeline_table[i] == pipeline)
			pipeline_table[i] = NULL;
}

static struct audio_pipeline *audio_pipeline_table_find(unsigned int id)
{
	if (id >= MAX_PIPELINES)
		return NULL;

	return pipeline_table[id];
}

static struct audio_element *audio_pipeline_element_find(struct audio_pipeline *pipeline, unsigned int type, unsigned int id)
{
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	unsigned int element_id = 0;
	int i, j;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];

			if (element->type == type) {
				if (element_id == id)
					return element;

				element_id++;
			}
		}
	}

	return NULL;
}

static struct audio_buffer *audio_pipeline_shared_buffer_find(unsigned shared_id)
{
	struct audio_pipeline *pipeline;
	struct audio_buffer *buffer;
	int i, j;

	for (i = 0; i < MAX_PIPELINES; i++) {
		pipeline  = audio_pipeline_table_find(i);
		if (pipeline == NULL)
			break;
		for ( j = 0; j < pipeline->buffers; j++) {
			buffer = &pipeline->buffer[j];
			if ((buffer->flags & AUDIO_BUFFER_FLAG_SHARED) && (buffer->shared_id == shared_id))
				return buffer;
		}
	}

	return NULL;
}

static void audio_pipeline_response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio_pipeline resp;

	if (m) {
		resp.type = HRPN_RESP_TYPE_AUDIO_PIPELINE;
		resp.status = status;
		mailbox_resp_send(m, &resp, sizeof(resp));
	}
}

int audio_pipeline_ctrl(struct hrpn_cmd_audio_pipeline *cmd, unsigned int len, struct mailbox *m)
{
	struct audio_pipeline *pipeline = NULL;
	struct audio_element *element = NULL;
	int rc = 0;

	/* search for matching pipeline id */
	if (len >= sizeof(struct hrpn_cmd_audio_pipeline_common))
		pipeline = audio_pipeline_table_find(cmd->u.common.pipeline.id);

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_AUDIO_PIPELINE_DUMP:
		if (len != sizeof(struct hrpn_cmd_audio_pipeline_dump))
			goto err;

		if (!pipeline)
			goto err;

		audio_pipeline_dump(pipeline);

		audio_pipeline_response(m, HRPN_RESP_STATUS_SUCCESS);

		break;

	default:
		if (pipeline && (len >= sizeof(struct hrpn_cmd_audio_element_common)))
			element = audio_pipeline_element_find(pipeline, cmd->u.element.u.common.element.type, cmd->u.element.u.common.element.id);

		rc = audio_element_ctrl(element, &cmd->u.element, len, m);

		break;
	}

	return rc;

err:
	audio_pipeline_response(m, HRPN_RESP_STATUS_ERROR);

	return -1;
}

static unsigned int audio_pipeline_count_input(struct audio_pipeline_config *config, unsigned int stage, unsigned int input)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	int count = 0;
	int i, j, k;

	for (i = stage; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			for (k = 0; k < element_config->inputs; k++) {
				if (element_config->input[k] == input)
					count++;
			}
		}
	}

	return count;
}

static unsigned int audio_pipeline_count_output(struct audio_pipeline_config *config, unsigned int stage, unsigned int output)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	int count = 0;
	int i, j, k;

	for (i = stage; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			for (k = 0; k < element_config->outputs; k++) {
				if (element_config->output[k] == output)
					count++;
			}
		}
	}

	return count;
}

static unsigned int audio_pipeline_count_buffers(struct audio_pipeline_config *config, unsigned int storage)
{
	int count = 0;
	int i;

	for (i = 0; i < config->buffers; i++) {
		if (config->buffer[i].storage == storage)
			count++;
	}

	return count;
}

static int audio_pipeline_config_check(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	unsigned int input, output;
	unsigned int buffer;
	int i, j, k;

	/* Sanity check pipeline configuration */

	/* Check that all elements inputs/outputs reference valid buffers */
	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			for (k = 0; k < element_config->inputs; k++) {
				if (element_config->input[k] >= config->buffers) {
					log_err("stage(%u), element(%u): input(%u) references invalid buffer(%u)\n", i, j, k, element_config->input[k]);
					goto err;
				}
			}

			for (k = 0; k < element_config->outputs; k++) {
				if (element_config->output[k] >= config->buffers) {
					log_err("stage(%u), element(%u): output(%u) references invalid buffer(%u)\n", i, j, k, element_config->output[k]);
					goto err;
				}
			}
		}
	}

	/* Check all buffers reference valid storage */
	for (i = 0; i < config->buffers; i++) {
		if (config->buffer[i].storage >= config->buffer_storage && !(config->buffer[i].flags & AUDIO_BUFFER_FLAG_SHARED_USER)) {
				log_err("buffer(%u) references invalid storage(%u)\n", i, config->buffer[i].storage);
				goto err;
		}

		/* Check all buffers are referenced by a single input and single output */
		input = audio_pipeline_count_input(config, 0, i);
		output = audio_pipeline_count_output(config, 0, i);

		if (input > 1) {
			log_warn("buffer(%u) referenced by %u inputs\n", i, input);
		}

		if (output > 1) {
			log_err("buffer(%u) referenced by %u outputs \n", i, output);
			goto err;
		}

		/* Check all buffers are referenced by one input and one output */
		if (!input)
			log_warn("buffer(%u) not referenced by any input\n", i);

		if (!output)
			log_warn("buffer(%u) not referenced by any output\n", i);
	}

	/* Check that no element input is connected to an ouput of a later stage */
	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			for (k = 0; k < element_config->inputs; k++) {
				output = audio_pipeline_count_output(config, i, element_config->input[k]);

				if (output) {
					log_err("stage(%u), element(%u): pipeline loop, input(%u) connected to same/later stage output\n", i, j, k);
					goto err;
				}
			}
		}
	}

	/* Check all buffer_storage is referenced by a buffer */
	for (i = 0; i < config->buffer_storage; i++) {
		buffer = audio_pipeline_count_buffers(config, i);

		if (!buffer)
			log_warn("storage(%u) not referenced\n", i);

		if (buffer > 1)
			log_warn("storage(%u) referenced by %u buffers\n", i, buffer);
	}

	/* Check the configuration of all elements */
	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			if (audio_element_check_config(element_config) < 0) {
				log_err("stage(%u), element(%u): invalid element configuration\n", i, j);
				goto err;
			}
		}
	}

	return 0;

err:
	return -1;
}

static int audio_pipeline_early_config_check(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	int i, j;

	if (config->stages > AUDIO_PIPELINE_MAX_STAGES) {
		log_err("pipeline: too many stages %u, max %u\n", config->stages, AUDIO_PIPELINE_MAX_STAGES);
		goto err;
	}

	if (config->buffers > AUDIO_PIPELINE_MAX_BUFFERS) {
		log_err("pipeline: too many buffers %u, max %u\n", config->buffers, AUDIO_PIPELINE_MAX_BUFFERS);
		goto err;
	}

	if (config->buffer_storage > AUDIO_PIPELINE_MAX_BUFFERS) {
		log_err("pipeline: too many storage %u, max %u\n", config->buffer_storage, AUDIO_PIPELINE_MAX_BUFFERS);
		goto err;
	}

	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		if (stage_config->elements > AUDIO_PIPELINE_MAX_ELEMENTS) {
			log_err("stage(%u): too many elements %u, max %u\n\r", i, stage_config->elements, AUDIO_PIPELINE_MAX_ELEMENTS);
			goto err;
		}

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			if (element_config->inputs > AUDIO_ELEMENT_MAX_INPUTS) {
				log_err("stage(%u), element(%u): too many inputs %u, max %u\n\r", i, j, element_config->inputs, AUDIO_ELEMENT_MAX_INPUTS);
				goto err;
			}

			if (element_config->outputs > AUDIO_ELEMENT_MAX_OUTPUTS) {
				log_err("stage(%u), element(%u): too many outputs %u, max %u\n\r", i, j, element_config->outputs, AUDIO_ELEMENT_MAX_OUTPUTS);
				goto err;
			}
		}
	}

	return 0;

err:
	return -1;
}

static unsigned int audio_buffer_storage_size(struct audio_pipeline_config *config)
{
	unsigned int size = 0;
	int i;

	for (i = 0; i < config->buffer_storage; i++) {
		if (!config->storage[i].base)
			size += config->storage[i].periods * config->period * sizeof(audio_sample_t);
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
	struct audio_buffer *buffer;
	unsigned int storage_id;
	audio_sample_t *base;
	unsigned int size;
	int i;

	stage_base = ((uint8_t *)pipeline + sizeof(struct audio_pipeline));
	element_base = ((uint8_t *)stage_base + audio_stage_size(config));
	element_data_base = ((uint8_t *)element_base + audio_element_size(config));

	buffer_base = ((uint8_t *)element_data_base + audio_element_data_size_total(config));
	buffer_storage_base = ((uint8_t *)buffer_base + audio_buffer_size(config));

	pipeline->buffer = (struct audio_buffer *)buffer_base;
	pipeline->buffers = config->buffers;

	for (i = 0; i < config->buffers; i++) {
		if (config->buffer[i].flags & AUDIO_BUFFER_FLAG_SHARED_USER) {
			buffer = audio_pipeline_shared_buffer_find(config->buffer[i].shared_id);
			if (buffer == NULL)
				log_err("Can't find shared buffer");
			else
				audio_buf_init(&pipeline->buffer[i], buffer->base, buffer->size, 0,
						config->buffer[i].flags, config->buffer[i].shared_id);
		} else {
			storage_id = config->buffer[i].storage;

			base = (audio_sample_t *)buffer_storage_base + audio_buffer_storage_off(config, storage_id);
			size = config->storage[storage_id].periods * config->period;

			if (config->buffer[i].flags & AUDIO_BUFFER_FLAG_SHARED)
				audio_buf_init(&pipeline->buffer[i], base, size, config->period,
						config->buffer[i].flags, config->buffer[i].shared_id);
			else
				audio_buf_init(&pipeline->buffer[i], base, size, 0, 0, 0);
		}
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

	log_info("enter\n");

	if (audio_pipeline_config_check(config) < 0)
		goto err;

	pipeline = audio_pipeline_alloc(config);
	if (!pipeline)
		goto err;

	audio_stage_init(pipeline, config);

	audio_pipeline_buffer_init(pipeline, config);

	log_info("done\n");

	return pipeline;

err:
	return NULL;
}

static void audio_pipeline_set_config(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	unsigned int next;
	int i, j, k;

	for (i = 0; i < config->stages; i++) {
		stage_config = &config->stage[i];

		for (j = 0; j < stage_config->elements; j++) {
			element_config = &stage_config->element[j];

			/* inherit default parameters if not overriden */
			if (!element_config->sample_rate)
				element_config->sample_rate = config->sample_rate;

			if (!element_config->period)
				element_config->period = config->period;

			/* Use 0 for default input, last + 1 */
			next = 0;
			for (k = 0; k < element_config->inputs; k++) {
				if (!element_config->input[k])
					element_config->input[k] = next;
				else
					next = element_config->input[k];

				next++;
			}

			/* Use 0 for default output, last + 1 */
			next = 0;
			for (k = 0; k < element_config->outputs; k++) {
				if (!element_config->output[k])
					element_config->output[k] = next;
				else
					next = element_config->output[k];

				next++;
			}
		}
	}

	/* Use 0, for default storage id (same as next free storage id) */
	j = 0;
	for (i = 0; i < config->buffers; i++) {
		if (!config->buffer[i].storage) {
			if ((config->buffer[i].flags & AUDIO_BUFFER_FLAG_SHARED_USER))
				config->buffer[i].storage = config->buffer[i].shared_id;
			else
				config->buffer[i].storage = j++;
		}
	}

	/* Use 0, for default storage size */
	for (i = 0; i < config->buffer_storage; i++)
		if (!config->storage[i].periods)
			config->storage[i].periods = STORAGE_DEFAULT_PERIODS;
}

struct audio_pipeline *audio_pipeline_init(struct audio_pipeline_config *config)
{
	struct audio_pipeline_stage_config *stage_config;
	struct audio_element_config *element_config;
	struct audio_pipeline *pipeline;
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	log_info("enter\n");

	if (audio_pipeline_early_config_check(config) < 0)
		goto err_config;

	audio_pipeline_set_config(config);

	pipeline = audio_pipeline_create(config);
	if (!pipeline)
		goto err_alloc;

	if (audio_pipeline_table_add(pipeline) < 0)
		goto err_add;

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

	log_info("done\n");

	return pipeline;

err_init:
	/* call exit for all already initialized elements */
	audio_pipeline_table_del(pipeline);

err_add:
	audio_pipeline_free(pipeline);

err_alloc:
err_config:
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

void audio_pipeline_reset(struct audio_pipeline *pipeline)
{
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];

			audio_element_reset(element);
		}
	}
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

	audio_pipeline_table_del(pipeline);
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

void audio_pipeline_stats(struct audio_pipeline *pipeline)
{
	struct audio_pipeline_stage *stage;
	struct audio_element *element;
	int i, j;

	for (i = 0; i < pipeline->stages; i++) {
		stage = &pipeline->stage[i];

		for (j = 0; j < stage->elements; j++) {
			element = &stage->element[j];

			audio_element_stats(element);
		}
	}
}
