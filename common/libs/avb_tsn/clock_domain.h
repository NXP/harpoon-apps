/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLOCK_DOMAIN_H_
#define _CLOCK_DOMAIN_H_

#include "genavb/control_clock_domain.h"

typedef enum {
    MEDIA_CLOCK_MASTER,
    MEDIA_CLOCK_SLAVE
} media_clock_role_t;

int genavb_clock_domain_set_source_stream(genavb_clock_domain_t domain, struct genavb_stream_params *stream_params);
int genavb_clock_domain_set_source_internal(genavb_clock_domain_t domain, genavb_clock_source_local_id_t local_id);
int genavb_clock_domain_set_role(media_clock_role_t role, genavb_clock_domain_t domain, struct genavb_stream_params *stream_params);
int genavb_clock_domain_init(struct genavb_handle *s_avb_handle);
void genavb_clock_domain_close(void);

#endif /* _CLOCK_DOMAIN_H_ */
