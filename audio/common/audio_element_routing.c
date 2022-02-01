/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element_routing.h"
#include "audio_element.h"


struct routing_output {
	unsigned int input;	/* input mapped to this output */
	struct audio_buffer *buf;
};

struct routing_element {
	unsigned int inputs;
	unsigned int outputs;
	struct audio_buffer **in;
	struct routing_output *out;
	struct audio_buffer silence; /* internal buffer with silence, used for "disconnected" outputs */
};

static int routing_element_run(struct audio_element *element)
{
	struct routing_element *routing = element->data;
	struct audio_buffer *in, *out;
	int i;

	/* Copy data from inputs to outputs */
	for (i = 0; i < routing->outputs; i++) {
		in = routing->in[routing->out[i].input];
		out = routing->out[i].buf;

		__audio_buf_copy(out, in, element->period);

		audio_buf_write_update(out, element->period);
	}

	/* Update all read pointers from inputs */
	for (i = 0; i < routing->inputs; i++) {
		in = routing->in[i];

		audio_buf_read_update(in, element->period);
	}

	return 0;
}

static void routing_element_exit(struct audio_element *element)
{
}

static void routing_element_dump(struct audio_element *element)
{
	struct routing_element *routing = element->data;
	int i;

	os_printf("routing(%p/%p)\n\r", routing, element);
	os_printf("  inputs: %u\n\r", routing->inputs);
	os_printf("  outputs: %u\n\r", routing->outputs);
	os_printf("  maping:\n\r");

	for (i = 0; i < routing->outputs; i++)
		os_printf("    %x => %x\n\r", routing->out[i].input, i);

	for (i = 0; i < routing->inputs + 1; i++)
		audio_buf_dump(routing->in[i]);

	for (i = 0; i < routing->outputs; i++)
		audio_buf_dump(routing->out[i].buf);
}

unsigned int routing_element_size(struct audio_element_config *config)
{
	unsigned int size;

	size = sizeof(struct routing_element);
	size += (config->inputs + 1) * sizeof(struct audio_buffer *);
	size += config->outputs * sizeof(struct routing_output);
	size += sizeof(int32_t) * config->period;

	return size;
}

int routing_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct routing_element *routing = element->data;
	int32_t *silence_storage;
	int32_t val;
	int i;

	element->run = routing_element_run;
	element->exit = routing_element_exit;
	element->dump = routing_element_dump;

	routing->inputs = config->inputs;
	routing->outputs = config->outputs;

	routing->in = (struct audio_buffer **)((uint8_t *)routing + sizeof(struct routing_element));
	routing->out = (struct routing_output *)((uint8_t *)routing->in + (config->inputs + 1) * sizeof(struct audio_buffer *));
	silence_storage = (int32_t *)((uint8_t *)routing->out + config->outputs * sizeof(struct routing_output));

	for (i = 0; i < routing->inputs; i++)
		routing->in[i] = &buffer[config->input[i]];

	routing->in[routing->inputs] = &routing->silence;

	for (i = 0; i < routing->outputs; i++) {
		/* initially all outputs are disconnected, i.e, connected to local silence input */
		routing->out[i].input = routing->inputs;

		routing->out[i].buf = &buffer[config->output[i]];
	}

	audio_buf_init(&routing->silence, silence_storage, element->period);

	val = 0;
	for (i = 0; i < element->period; i++)
		audio_buf_write(&routing->silence, &val, 1);

	routing_element_dump(element);

	return 0;
}
