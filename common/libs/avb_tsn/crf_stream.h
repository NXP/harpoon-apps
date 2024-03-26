/*
 * Copyright 2017, 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CRF_STREAM_H_
#define _CRF_STREAM_H_

#include "clock_domain.h"
#include "genavb/streaming.h"

typedef struct {
	struct genavb_stream_params stream_params;
	struct genavb_stream_handle *stream_handle;

	unsigned int batch_size_ns;
	unsigned int cur_batch_size;
} aar_crf_stream_t;

int crf_connect(aar_crf_stream_t *crf, media_clock_role_t role, genavb_clock_domain_t clock_domain, struct genavb_stream_params *stream_params);
void crf_disconnect(aar_crf_stream_t *crf);

#endif /* _CLOCK_DOMAIN_H_ */
