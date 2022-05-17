/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/semaphore.h"

#include "audio_element_routing.h"
#include "audio_element.h"
#include "audio_format.h"
#include "hrpn_ctrl.h"
#include "hlog.h"
#include "mailbox.h"

struct routing_output {
	unsigned int input;	/* input mapped to this output */
	struct audio_buffer *buf;
};

struct routing_element {
	unsigned int inputs;
	unsigned int outputs;
	struct audio_buffer **in;
	struct routing_output *out;
	os_sem_t semaphore;
	struct audio_buffer silence; /* internal buffer with silence, used for "disconnected" outputs */
};

static void routing_element_response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio_element_routing resp;

	resp.type = HRPN_RESP_TYPE_AUDIO_ELEMENT_ROUTING;
	resp.status = status;
	mailbox_resp_send(m, &resp, sizeof(resp));
}

void routing_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element_routing *cmd, unsigned int len, struct mailbox *m)
{
	struct routing_element *routing;
	unsigned int output, input;

	if (!element)
		goto err;

	if (element->type != AUDIO_ELEMENT_ROUTING)
		goto err;

	routing = element->data;

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT:
		if ((len != sizeof(struct hrpn_cmd_audio_element_routing_connect)))
			goto err;

		if (cmd->u.connect.output >= routing->outputs)
			goto err;

		if (cmd->u.connect.input >= routing->inputs)
			goto err;

		output = cmd->u.connect.output;
		input = cmd->u.connect.input;

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT:
		if ((len != sizeof(struct hrpn_cmd_audio_element_routing_disconnect)))
			goto err;

		if (cmd->u.disconnect.output >= routing->outputs)
			goto err;

		output = cmd->u.disconnect.output;
		input = routing->inputs;

		break;

	default:
		goto err;
		break;
	}

	os_sem_take(&routing->semaphore, 0, OS_SEM_TIMEOUT_MAX);

	routing->out[output].input = input;

	os_sem_give(&routing->semaphore, 0);

	routing_element_response(m, HRPN_RESP_STATUS_SUCCESS);

	return;

err:
	routing_element_response(m, HRPN_RESP_STATUS_ERROR);
}

static int routing_element_run(struct audio_element *element)
{
	struct routing_element *routing = element->data;
	struct audio_buffer *in, *out;
	int i;

	os_sem_take(&routing->semaphore, 0, OS_SEM_TIMEOUT_MAX);

	/* Copy data from inputs to outputs */
	for (i = 0; i < routing->outputs; i++) {
		in = routing->in[routing->out[i].input];
		out = routing->out[i].buf;

		__audio_buf_copy(out, in, element->period);

		audio_buf_write_update(out, element->period);
	}

	os_sem_give(&routing->semaphore, 0);

	/* Update all read pointers from inputs */
	for (i = 0; i < routing->inputs; i++) {
		in = routing->in[i];

		audio_buf_read_update(in, element->period);
	}

	return 0;
}

static void routing_element_reset(struct audio_element *element)
{
	struct routing_element *routing = element->data;
	int i;

	for (i = 0; i < routing->outputs; i++)
		audio_buf_reset(routing->out[i].buf);
}

static void routing_element_exit(struct audio_element *element)
{
	struct routing_element *routing = element->data;

	os_sem_destroy(&routing->semaphore);
}

static void routing_element_dump(struct audio_element *element)
{
	struct routing_element *routing = element->data;
	int i;

	log_info("routing(%p/%p)\n", routing, element);
	log_info("  inputs: %u\n", routing->inputs);
	log_info("  outputs: %u\n", routing->outputs);
	log_info("  maping:\n");

	for (i = 0; i < routing->outputs; i++)
		log_info("    %x => %x\n", routing->out[i].input, i);

	for (i = 0; i < routing->inputs + 1; i++)
		audio_buf_dump(routing->in[i]);

	for (i = 0; i < routing->outputs; i++)
		audio_buf_dump(routing->out[i].buf);
}

int routing_element_check_config(struct audio_element_config *config)
{
	if (!config->inputs) {
		log_err("routing: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (!config->outputs) {
		log_err("routing: invalid outputs: %u\n", config->outputs);
		goto err;
	}

	return 0;

err:
	return -1;
}

unsigned int routing_element_size(struct audio_element_config *config)
{
	unsigned int size;

	size = sizeof(struct routing_element);
	size += (config->inputs + 1) * sizeof(struct audio_buffer *);
	size += config->outputs * sizeof(struct routing_output);
	size += sizeof(audio_sample_t) * config->period;

	return size;
}

int routing_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct routing_element *routing = element->data;
	audio_sample_t *silence_storage;
	audio_sample_t val;
	int i;

	if (os_sem_init(&routing->semaphore, 1))
		goto err;

	element->run = routing_element_run;
	element->reset = routing_element_reset;
	element->exit = routing_element_exit;
	element->dump = routing_element_dump;

	routing->inputs = config->inputs;
	routing->outputs = config->outputs;

	routing->in = (struct audio_buffer **)((uint8_t *)routing + sizeof(struct routing_element));
	routing->out = (struct routing_output *)((uint8_t *)routing->in + (config->inputs + 1) * sizeof(struct audio_buffer *));
	silence_storage = (audio_sample_t *)((uint8_t *)routing->out + config->outputs * sizeof(struct routing_output));

	for (i = 0; i < routing->inputs; i++)
		routing->in[i] = &buffer[config->input[i]];

	routing->in[routing->inputs] = &routing->silence;

	for (i = 0; i < routing->outputs; i++) {
		/* initially all outputs are disconnected, i.e, connected to local silence input */
		routing->out[i].input = routing->inputs;

		routing->out[i].buf = &buffer[config->output[i]];
	}

	audio_buf_init(&routing->silence, silence_storage, element->period);

	val = AUDIO_SAMPLE_SILENCE;
	for (i = 0; i < element->period; i++)
		audio_buf_write(&routing->silence, &val, 1);

	routing_element_dump(element);

	return 0;

err:
	return -1;
}
