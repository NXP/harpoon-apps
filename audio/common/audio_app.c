/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_app.h"
#include "rpmsg.h"

#define EPT_ADDR	(30)

int audio_app_ctrl_send(void *ctrl_handle, void *data, uint32_t len)
{
	struct rpmsg_ept *ept = (struct rpmsg_ept *)ctrl_handle;

	return rpmsg_send(ept, data, len);
}

int audio_app_ctrl_recv(void *ctrl_handle, void *data, uint32_t *len)
{
	struct rpmsg_ept *ept = (struct rpmsg_ept *)ctrl_handle;

	return rpmsg_recv(ept, data, len);
}

void *audio_app_ctrl_init(void)
{
	void *ctrl_handle = (void *)rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw");

	return ctrl_handle;
}