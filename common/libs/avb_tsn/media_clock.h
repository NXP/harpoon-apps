/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MEDIA_CLOCK_H_
#define _MEDIA_CLOCK_H_

#include <genavb/genavb.h>
#include "crf_stream.h"
#include "genavb/control_clock_domain.h"

int init_media_clock_source(aar_crf_stream_t *crf, genavb_clock_domain_t clock_domain, struct genavb_stream_params *listener_stream_params);

#endif /* _MEDIA_CLOCK_H_ */
