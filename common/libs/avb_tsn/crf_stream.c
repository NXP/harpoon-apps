/*
 * Copyright 2017, 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <genavb/genavb.h>

#include "genavb/control_clock_domain.h"
#include "log.h"
#include "genavb.h"
#include "crf_stream.h"
#include "clock_domain.h"

static inline unsigned int genavb_batch_size(unsigned int batch_size_ns, struct genavb_stream_params *params)
{
	return (((uint64_t)batch_size_ns * avdecc_fmt_sample_rate(&params->format) * avdecc_fmt_sample_size(&params->format) + NSECS_PER_SEC - 1) / NSECS_PER_SEC);
}

int crf_connect(aar_crf_stream_t *crf, media_clock_role_t role, genavb_clock_domain_t clock_domain, struct genavb_stream_params *stream_params)
{
	struct genavb_handle *genavb_handle = get_genavb_handle();
	int rc;

	if (clock_domain >= GENAVB_CLOCK_DOMAIN_MAX || clock_domain < GENAVB_CLOCK_DOMAIN_0) {
		ERR("Invalid domain_index (%u)\n", clock_domain);
		goto err;
	}

	if (stream_params)
		stream_params->clock_domain = clock_domain;

	if (!crf) {
		ERR("CRF stream not found for domain_index (%u)\n", clock_domain);
		goto err;
	}

	if (crf->stream_handle) {
		ERR("CRF stream already connected for domain_index (%u)\n", clock_domain);
		goto err;
	}

	rc = genavb_clock_domain_set_role(role, clock_domain, stream_params);
	if (rc != GENAVB_SUCCESS) {
		ERR("clock_domain_set_role failed, rc = %d\n", rc);
		goto err;
	}

	/* Update CRF stream params */
	if (stream_params)
		memcpy(&crf->stream_params, stream_params, sizeof(*stream_params));

	crf->cur_batch_size = genavb_batch_size(crf->batch_size_ns, &crf->stream_params);

	rc = genavb_stream_create(genavb_handle, &crf->stream_handle, &crf->stream_params, &crf->cur_batch_size, 0);
	if (rc != GENAVB_SUCCESS) {
		ERR("genavb_stream_create failed, rc = %d\n", rc);
		goto err;
	}

	return 0;

err:
	return -1;
}

void crf_disconnect(aar_crf_stream_t *crf)
{
	if (!crf) {
		ERR("CRF stream not found\n");
		return;
	}

	if (!crf->stream_handle) {
		ERR("CRF stream already disconnected for domain_index (%u)\n", crf->stream_params.clock_domain);
		return;
	}

	if (genavb_stream_destroy(crf->stream_handle) != GENAVB_SUCCESS)
		ERR("genavb_stream_destroy error\n");

	crf->stream_handle = NULL;
}
