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

struct ethernet_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;
};

void ethernet_stats(void *priv)
{
	struct ethernet_ctx *ctx = priv;

	(void)ctx;
	log_info("not implemented\n");
}

int ethernet_run(void *priv, struct event *e)
{
	struct ethernet_ctx *ctx = priv;

	(void)ctx;
	log_info("not implemented\n");

	return 0;
}

void *ethernet_init(void *parameters)
{
	struct industrial_config *cfg = parameters;
	struct ethernet_ctx *ctx;

	ctx = os_malloc(sizeof(struct ethernet_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");

		goto exit;
	}

	memset(ctx, 0, sizeof(struct ethernet_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	log_info("not implemented\n");

exit:
	return ctx;
}

void ethernet_exit(void *priv)
{
	struct ethernet_ctx *ctx = priv;

	os_free(ctx);

	log_info("end\n");
}
