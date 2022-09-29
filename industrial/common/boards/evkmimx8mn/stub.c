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

STUB_INDUSTRIAL_USE_CASE(can);

STUB_INDUSTRIAL_USE_CASE(ethernet_sdk_enet_loopback);
