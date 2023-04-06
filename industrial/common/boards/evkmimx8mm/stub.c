/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stub.h"

void *can_init_loopback(void *parameters)
{
	log_err("not supported\n");
	return NULL;
}

void *can_init_interrupt(void *parameters)
{
	log_err("not supported\n");
	return NULL;
}

void *can_init_pingpong(void *parameters)
{
	log_err("not supported\n");
	return NULL;
}

void *flexcan_init(void *parameters)
{
	log_err("not supported\n");
	return NULL;
}

int flexcan_run(void *priv, struct event *e)
{
	log_err("not supported\n");
	return -1;
}

void flexcan_exit(void *priv)
{
	log_err("not supported\n");
	return;
}

void flexcan_stats(void *priv)
{
	log_err("not supported\n");
	return;
}

STUB_INDUSTRIAL_USE_CASE(can);

STUB_INDUSTRIAL_USE_CASE(ethernet_sdk_enet_loopback);
