/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_element.h"
#include "audio_pipeline.h"
#include "hrpn_ctrl.h"
#include "hlog.h"
#include "mailbox.h"

static void audio_element_response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio_element resp;

	if (m) {
		resp.type = HRPN_RESP_TYPE_AUDIO_ELEMENT;
		resp.status = status;
		mailbox_resp_send(m, &resp, sizeof(resp));
	}
}

int audio_element_ctrl(struct audio_element *element, struct hrpn_cmd_audio_element *cmd, unsigned int len, struct mailbox *m)
{
	int rc = 0;

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_DUMP:
		if (len != sizeof(struct hrpn_cmd_audio_element_dump))
			goto err;

		if (!element)
			goto err;

		audio_element_dump(element);

		audio_element_response(m, HRPN_RESP_STATUS_SUCCESS);

		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT:
		rc = routing_element_ctrl(element, &cmd->u.routing, len, m);
		break;

	case HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ENABLE:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_DISABLE:
		rc = pll_element_ctrl(element, &cmd->u.pll, len, m);
		break;

	default:
		goto err;
		break;
	}

	return rc;

err:
	audio_element_response(m, HRPN_RESP_STATUS_ERROR);

	return -1;
}

void audio_element_exit(struct audio_element *element)
{
	element->exit(element);
}

void audio_element_dump(struct audio_element *element)
{
	if (element->dump)
		element->dump(element);
}

void audio_element_stats(struct audio_element *element)
{
	if (element->stats)
		element->stats(element);
}

int audio_element_check_config(struct audio_element_config *config)
{
	int rc;

	switch (config->type) {
	case AUDIO_ELEMENT_DTMF_SOURCE:
		rc = dtmf_element_check_config(config);
		break;

	case AUDIO_ELEMENT_PLL:
		rc = pll_element_check_config(config);
		break;

	case AUDIO_ELEMENT_ROUTING:
		rc = routing_element_check_config(config);
		break;

	case AUDIO_ELEMENT_SAI_SINK:
		rc = sai_sink_element_check_config(config);
		break;

	case AUDIO_ELEMENT_SAI_SOURCE:
		rc = sai_source_element_check_config(config);
		break;

	case AUDIO_ELEMENT_SINE_SOURCE:
		rc = sine_element_check_config(config);
		break;

	default:
		rc = -1;
		break;
	}

	return rc;
}

unsigned int audio_element_data_size(struct audio_element_config *config)
{
	unsigned int size;

	switch (config->type) {
	case AUDIO_ELEMENT_DTMF_SOURCE:
		size = dtmf_element_size(config);
		break;

	case AUDIO_ELEMENT_PLL:
		size = pll_element_size(config);
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

int audio_element_init(struct audio_element *element, struct audio_element_config *config, struct audio_buffer *buffer)
{
	int rc;

	log_info("enter, type %d\n", config->type);

	element->type = config->type;
	element->sample_rate = config->sample_rate;
	element->period = config->period;

	switch (config->type) {
	case AUDIO_ELEMENT_DTMF_SOURCE:
		rc = dtmf_element_init(element, config, buffer);
		break;

	case AUDIO_ELEMENT_PLL:
		rc = pll_element_init(element, config, buffer);
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

	log_info("done\n");

	return rc;
}
