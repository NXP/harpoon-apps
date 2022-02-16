/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/math.h"
#include "os/stdio.h"

#include "audio_element_sine.h"
#include "audio_element.h"
#include "audio_buffer.h"

struct sine_element {
	struct audio_buffer *out;

	unsigned int phase;

	double amplitude; /* sine amplitude, ]0, 1] */
	double dphase; /* change of phase per sample */
};

static int sine_element_run(struct audio_element *element)
{
	struct sine_element *sine = element->data;
        double v;
	int32_t w;
	int i;

	for (i = 0; i < element->period; i++) {
		v = sine->amplitude * sin(sine->phase * sine->dphase);
		w = (int32_t)(v * ((1 << 30) - 1));

		__audio_buf_write(sine->out, i, &w, 1);
		sine->phase++;
	}

	audio_buf_write_update(sine->out, element->period);

	return 0;
}

static void sine_element_reset(struct audio_element *element)
{
	struct sine_element *sine = element->data;

	audio_buf_reset(sine->out);
}

static void sine_element_exit(struct audio_element *element)
{
}

static void sine_element_dump(struct audio_element *element)
{
	struct sine_element *sine = element->data;

	os_printf("sine(%p/%p)\n\r", sine, element);
	os_printf("  phase: %u, amplitude: %f\n\r", sine->phase, sine->amplitude);
	audio_buf_dump(sine->out);
}

unsigned int sine_element_size(struct audio_element_config *config)
{
	return sizeof(struct sine_element);
}

int sine_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct sine_element *sine = element->data;

	/* some sanity checks */
	if (config->inputs)
		goto err;

	if (config->outputs != 1)
		goto err;

	element->run = sine_element_run;
	element->reset = sine_element_reset;
	element->exit = sine_element_exit;
	element->dump = sine_element_dump;

	sine->dphase = 2.0 * M_PI * config->u.sine.freq / config->sample_rate;
	sine->amplitude = config->u.sine.amplitude;
	sine->phase = 0;
	sine->out = &buffer[config->output[0]];

	sine_element_dump(element);

	return 0;

err:
	return -1;
}
