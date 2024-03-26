/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include <genavb/genavb.h>

#include "clock_domain.h"
#include "log.h"

static struct genavb_control_handle *s_clk_handle;

int genavb_clock_domain_init(struct genavb_handle *s_avb_handle)
{
	int rc;

	rc = genavb_control_open(s_avb_handle, &s_clk_handle, GENAVB_CTRL_CLOCK_DOMAIN);
	if (rc != GENAVB_SUCCESS) {
		ERR("genavb_control_open(GENAVB_CTRL_CLOCK_DOMAIN)  failed: %s\n", genavb_strerror(rc));
		return -1;
	}

	return 0;
}

static int clock_domain_set_source(struct genavb_msg_clock_domain_set_source *set_source)
{
	struct genavb_msg_clock_domain_response set_source_rsp;
	unsigned int msg_len = sizeof(*set_source);
	unsigned int msg_type = GENAVB_MSG_CLOCK_DOMAIN_SET_SOURCE;
	int rc;

	rc = genavb_control_send_sync(s_clk_handle, &msg_type, set_source, msg_len, &set_source_rsp, &msg_len, 1000);
	if ((rc == GENAVB_SUCCESS) && (msg_type == GENAVB_MSG_CLOCK_DOMAIN_RESPONSE))
		rc = set_source_rsp.status;

	return rc;
}

int genavb_clock_domain_set_source_internal(genavb_clock_domain_t domain,
							genavb_clock_source_local_id_t local_id)
{
	struct genavb_msg_clock_domain_set_source set_source;

	set_source.domain = domain;
	set_source.source_type = GENAVB_CLOCK_SOURCE_TYPE_INTERNAL;
	set_source.local_id = local_id;

	return clock_domain_set_source(&set_source);
}

int genavb_clock_domain_set_source_stream(genavb_clock_domain_t domain,
						struct genavb_stream_params *stream_params)
{
	struct genavb_msg_clock_domain_set_source set_source;

	set_source.domain = domain;
	set_source.source_type = GENAVB_CLOCK_SOURCE_TYPE_INPUT_STREAM;
	memcpy(set_source.stream_id, stream_params->stream_id, 8);

	return clock_domain_set_source(&set_source);
}

int genavb_clock_domain_set_role(media_clock_role_t role, genavb_clock_domain_t domain,
					struct genavb_stream_params *stream_params)
{
	int rc = 0;

	/* Master */
	if (role == MEDIA_CLOCK_MASTER) {
		/* If possible try to configure with internal HW source */
		if (genavb_clock_domain_set_source_internal(domain, GENAVB_CLOCK_SOURCE_AUDIO_CLK) != GENAVB_SUCCESS) {
			INF("cannot set clock source to internal audio clock\n");

			/* Fallback */
			rc = genavb_clock_domain_set_source_internal(domain, GENAVB_CLOCK_SOURCE_PTP_CLK);
			if (rc != GENAVB_SUCCESS) {
				ERR("cannot set clock source to PTP based clock, rc = %d\n", rc);
				goto exit;
			}
			INF("successful fallback to PTP based clock\n");
		}
	}
	/* Slave */
	else {
		if (!stream_params) {
			ERR("slave role requires a stream argument\n");
			goto exit;
		}

		rc = genavb_clock_domain_set_source_stream(domain, stream_params);
		if (rc != GENAVB_SUCCESS) {
			ERR("clock_domain_set_source_stream error, rc = %d\n", rc);
			goto exit;
		}

		INF("clock source setup to stream ID" STREAM_STR_FMT "\n", STREAM_STR(stream_params->stream_id));
	}

exit:
	return rc;
}
