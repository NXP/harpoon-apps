/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include <genavb/genavb.h>

#include "clock_domain.h"
#include "crf_stream.h"
#include "media_clock.h"
#include "log.h"

int init_media_clock_source(aar_crf_stream_t *crf, genavb_clock_domain_t clock_domain, struct genavb_stream_params *listener_stream_params)
{
	int rc;

	if (!crf) {
		log_err("init_media_clock_source: current crf config is null\n");
		goto err;
	}

	/* If CRF is already connected, do nothing: the media clock domain source should be properly set. */
	if (crf->stream_handle)
		goto exit;

	if (!listener_stream_params) {
		/* If connecting an AVTP talker stream without any previous CRF connection: set as media clock master. */
		rc = genavb_clock_domain_set_role(MEDIA_CLOCK_MASTER, clock_domain, NULL);
		if (rc != GENAVB_SUCCESS) {
			log_err("clock_domain_set_role failed: %s\n", genavb_strerror(rc));
			goto err;
		}
	} else {
		/* If connecting an AVTP listener stream without any previous CRF connection: set as media clock slave to that stream. */
		rc = genavb_clock_domain_set_role(MEDIA_CLOCK_SLAVE, clock_domain, listener_stream_params);
		if (rc != GENAVB_SUCCESS) {
			log_err("clock_domain_set_role failed: %s\n", genavb_strerror(rc));
			goto err;
		}
	}

exit:
	return 0;

err:
	return -1;
}
