/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ethernet.h"
#include "hrpn_ctrl.h"
#include "industrial.h"

static void ethernet_response(struct rpmsg_ept *ept, uint32_t status)
{
	struct hrpn_resp_industrial resp;

	resp.type = HRPN_RESP_TYPE_INDUSTRIAL;
	resp.status = status;
	rpmsg_send(ept, &resp, sizeof(resp));
}

void ethernet_ctrl(struct hrpn_cmd_ethernet *cmd, unsigned int len,
		   struct rpmsg_ept *ept, void *priv)
{
	struct ethernet_ctx *ctx = priv;

	(void)ctx;

	switch (cmd->u.common.type) {

	default:
		ethernet_response(ept, HRPN_RESP_STATUS_ERROR);
		break;
	}

	return;
}
