/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/math.h"
#include "os/stdint.h"
#include "os/string.h"

#include "audio_element_dtmf.h"
#include "audio_element.h"
#include "log.h"

#define USEC_PER_SEC	1000000

struct dtmf_element {
	struct audio_buffer *out;

	unsigned int sample_rate;

	double amplitude;
	double dphase1;
	double dphase2;

	unsigned int phase;	/* current phase, in sample units */
	unsigned int dtmf_samples; /* number of samples to play per dtmf */
	unsigned int dtmf_pause_samples; /* number of samples of silence to play between each dtmf */
	unsigned int sequence_pause_samples; /* number of samples of silence to play between each dtmf sequence */

	char *sequence;	/* dtmf sequence */
	unsigned int sequence_size; /* dtmf sequence size */
	unsigned int sequence_id; /* current dtmf in sequence */

	unsigned int state;
};

enum {
	DTMF_PLAYING = 0,
	DTMF_PAUSE,
	DTMF_SEQUENCE_PAUSE,
};

static void dtmf_generate_sinewave(struct dtmf_element *dtmf, unsigned int samples)
{
	double v;
	int32_t w;
	int i;

	for (i = 0; i < samples; i++) {
		v = dtmf->amplitude * (sin((i + dtmf->phase) * dtmf->dphase1) + sin((i + dtmf->phase) * dtmf->dphase2)) / 2.0;
		w = (int32_t)(v * ((1 << 30) - 1));

		__audio_buf_write(dtmf->out, i, &w, 1);
	}

	audio_buf_write_update(dtmf->out, samples);

	dtmf->phase += samples;
}

static void dtmf_generate_silence(struct dtmf_element *dtmf, unsigned int samples)
{
	int i;
	int32_t silence = 0;

	for (i = 0; i < samples; i++)
		__audio_buf_write(dtmf->out, i, &silence, 1);

	audio_buf_write_update(dtmf->out, samples);

	dtmf->phase += samples;
}

static void dtmf_get_freqs(char key, unsigned int *freq1, unsigned int *freq2)
{
	switch (key) {
	case '1':
	case '2':
	case '3':
	case 'A':
	default:
		/* 697 Hz */
		*freq1 = 697;
		break;

	case '4':
	case '5':
	case '6':
	case 'B':
		/* 770 Hz */
		*freq1 = 770;
		break;

	case '7':
	case '8':
	case '9':
	case 'C':
		/* 852 Hz */
		*freq1 = 852;
		break;

	case '*':
	case '0':
	case '#':
	case 'D':
		/* 941 Hz */
		*freq1 = 941;
		break;
	}

	switch (key) {
	case '1':
	case '4':
	case '7':
	case '*':
		/* 1209 Hz */
		*freq2 = 1209;
		break;

	case '2':
	case '5':
	case '8':
	case '0':
		/* 1336 Hz */
		*freq2 = 1336;
		break;

	case '3':
	case '6':
	case '9':
	case '#':
		/* 1477 Hz */
		*freq2 = 1477;
		break;

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	default:
		/* 1633 Hz */
		*freq2 = 1633;
		break;
	}
}

static int dtmf_element_run(struct audio_element *element)
{
	struct dtmf_element *dtmf = element->data;
	unsigned int freq1, freq2;

	switch (dtmf->state) {
	case DTMF_PLAYING:
		dtmf_generate_sinewave(dtmf, element->period);

		if (dtmf->phase >= dtmf->dtmf_samples) {
			dtmf->sequence_id++;
			dtmf->phase = 0;

			if (dtmf->sequence_id == dtmf->sequence_size) {
				dtmf->sequence_id = 0;
				dtmf->state = DTMF_SEQUENCE_PAUSE;
			} else {
				dtmf->state = DTMF_PAUSE;
			}

			dtmf_get_freqs(dtmf->sequence[dtmf->sequence_id], &freq1, &freq2);

			dtmf->dphase1 = 2.0 * M_PI * freq1 / dtmf->sample_rate;
			dtmf->dphase2 = 2.0 * M_PI * freq2 / dtmf->sample_rate;
		}

		break;

	case DTMF_PAUSE:
		dtmf_generate_silence(dtmf, element->period);

		if (dtmf->phase >= dtmf->dtmf_pause_samples) {
			dtmf->phase = 0;
			dtmf->state = DTMF_PLAYING;
		}

		break;

	case DTMF_SEQUENCE_PAUSE:
		dtmf_generate_silence(dtmf, element->period);

		if (dtmf->phase >= dtmf->sequence_pause_samples) {
			dtmf->phase = 0;
			dtmf->state = DTMF_PLAYING;
		}

		break;

	default:
		break;
	}

	return 0;
}

static void dtmf_element_reset(struct audio_element *element)
{
	struct dtmf_element *dtmf = element->data;

	audio_buf_reset(dtmf->out);
}

static void dtmf_element_exit(struct audio_element *element)
{
}

static void dtmf_element_dump(struct audio_element *element)
{
	struct dtmf_element *dtmf = element->data;

	log_info("dmtf(%p/%p), state: %u\n", dtmf, element, dtmf->state);
	log_info("sequence: %s, %u\n", dtmf->sequence, dtmf->sequence_id);
	audio_buf_dump(dtmf->out);
}

int dtmf_element_check_config(struct audio_element_config *config)
{
	if (config->inputs) {
		log_err("dtmf: invalid inputs: %u\n", config->inputs);
		goto err;
	}

	if (config->outputs != 1) {
		log_err("dtmf: invalid outputs: %u\n", config->outputs);
		goto err;
	}

	if ((config->u.sine.amplitude <= 0.0) || (config->u.sine.amplitude > 1.0)) {
		log_err("dtmf: invalid amplitude: %f\n", config->u.dtmf.amplitude);
		goto err;
	}

	if (strlen(config->u.dtmf.sequence) < 1) {
		log_err("dtmf: invalid sequence length: %u\n", strlen(config->u.dtmf.sequence));
		goto err;
	}

	return 0;

err:
	return -1;
}

unsigned int dtmf_element_size(struct audio_element_config *config)
{
	return sizeof(struct dtmf_element);
}

int dtmf_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	struct dtmf_element *dtmf = element->data;
	unsigned int freq1, freq2;

	element->run = dtmf_element_run;
	element->reset = dtmf_element_reset;
	element->exit = dtmf_element_exit;
	element->dump = dtmf_element_dump;

	dtmf->sample_rate = config->sample_rate;

	dtmf->sequence = config->u.dtmf.sequence;
	dtmf->sequence_size = strlen(dtmf->sequence);
	dtmf->sequence_id = 0;

	dtmf->dtmf_samples = ((uint64_t)config->sample_rate * config->u.dtmf.us) / USEC_PER_SEC;
	dtmf->dtmf_pause_samples = ((uint64_t)config->sample_rate * config->u.dtmf.pause_us) / USEC_PER_SEC;
	dtmf->sequence_pause_samples = ((uint64_t)config->sample_rate * config->u.dtmf.sequence_pause_us) / USEC_PER_SEC;

	dtmf->amplitude = config->u.dtmf.amplitude;
	dtmf->out = &buffer[config->output[0]];

	dtmf_get_freqs(dtmf->sequence[dtmf->sequence_id], &freq1, &freq2);

	dtmf->dphase1 = 2.0 * M_PI * freq1 / dtmf->sample_rate;
	dtmf->dphase2 = 2.0 * M_PI * freq2 / dtmf->sample_rate;

	dtmf->state = DTMF_PLAYING;

	dtmf_element_dump(element);

	return 0;
}
