/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "hlog.h"
#include "industrial.h"

#include "os/stdlib.h"
#include "os/string.h"

struct can_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;
};

void can_stats(void *priv)
{
	struct can_ctx *ctx = priv;

	(void)ctx;
	log_info("not implemented\n");
}

int can_run(void *priv, struct event *e)
{
	struct can_ctx *ctx = priv;

	(void)ctx;
	log_info("not implemented\n");

	return 0;
}

void *can_init(void *parameters)
{
	struct industrial_config *cfg = parameters;
	struct can_ctx *ctx;

	ctx = os_malloc(sizeof(struct can_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");

		goto exit;
	}

	memset(ctx, 0, sizeof(struct can_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	log_info("not implemented\n");

exit:
	return ctx;
}

void can_exit(void *priv)
{
	struct can_ctx *ctx = priv;

	os_free(ctx);

	log_info("end\n");
}
