/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hlog.h"
#include "hrpn_ctrl.h"
#include "industrial.h"

#define STUB_INDUSTRIAL_USE_CASE(x) \
\
void *x##_init(void *parameters) \
{ \
	log_err("not supported\n"); \
 \
	return NULL; \
} \
void x##_exit(void *priv) \
{ \
	(void)priv; \
	log_err("not supported\n"); \
} \
void x##_pre_exit(void *priv) \
{ \
	(void)priv; \
	log_err("not supported\n"); \
} \
\
void x##_stats(void *priv) \
{ \
	(void)priv; \
	log_err("not supported\n"); \
} \
\
int x##_run(void *priv, struct event *e) \
{ \
	(void)priv; \
	log_err("not supported\n"); \
 \
	 return -1; \
} \
void x##_ctrl(struct hrpn_cmd_ethernet *cmd, unsigned int len, \
	       struct mailbox *mb, void *priv) \
{ \
	log_err("not supported\n"); \
}

/*
 * Usage:
 *
 * STUB_INDUSTRIAL_USE_CASE(<component>);
 *
 * to stub the entry functions for a specific <component> that
 * is not supported by some hardware.
 */
