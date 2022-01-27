/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/stdio.h"

#include "audio_element.h"

unsigned int audio_element_data_size(struct audio_element_config *config)
{
	unsigned int size;

	switch (config->type) {
	case AUDIO_ELEMENT_DTMF_SOURCE:
		size = dtmf_element_size(config);
		break;

	case AUDIO_ELEMENT_ROUTING:
		size = routing_element_size(config);
		break;

	case AUDIO_ELEMENT_SAI_SINK:
		size = sai_sink_element_size(config);
		break;

	case AUDIO_ELEMENT_SAI_SOURCE:
		size = sai_source_element_size(config);
		break;

	case AUDIO_ELEMENT_SINE_SOURCE:
		size = sine_element_size(config);
		break;

	default:
		size = 0;
		break;
	}

	return size;
}

void audio_element_dump(struct audio_element *element)
{
	if (element->dump)
		element->dump(element);
}

int audio_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	int rc;

	os_printf("%s: enter, type %d\n\r", __func__, config->type);

	element->sample_rate = config->sample_rate;
	element->period = config->period;

	switch (config->type) {
	case AUDIO_ELEMENT_DTMF_SOURCE:
		rc = dtmf_element_init(element, config, buffer);
		break;

	case AUDIO_ELEMENT_ROUTING:
		rc = routing_element_init(element, config, buffer);
		break;

	case AUDIO_ELEMENT_SAI_SINK:
		rc = sai_sink_element_init(element, config, buffer);
		break;

	case AUDIO_ELEMENT_SAI_SOURCE:
		rc = sai_source_element_init(element, config, buffer);
		break;

	case AUDIO_ELEMENT_SINE_SOURCE:
		rc = sine_element_init(element, config, buffer);
		break;

	default:
		rc = -1;
		break;
	}

	os_printf("%s: done\n\r", __func__);

	return rc;
}

void audio_element_exit(struct audio_element *element)
{
	element->exit(element);
}
