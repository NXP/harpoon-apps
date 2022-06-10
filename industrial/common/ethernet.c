/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ethernet.h"
#include "app_board.h"
#include "hrpn_ctrl.h"
#include "industrial.h"

#include "os/string.h"

static void ethernet_response(struct mailbox *mb, uint32_t status)
{
	struct hrpn_resp_industrial resp;

	resp.type = HRPN_RESP_TYPE_INDUSTRIAL;
	resp.status = status;
	mailbox_resp_send(mb, &resp, sizeof(resp));
}

void ethernet_ctrl(struct hrpn_cmd_ethernet *cmd, unsigned int len,
		   struct mailbox *mb, void *priv)
{
	struct ethernet_ctx *ctx = priv;

	(void)ctx;

	switch (cmd->u.common.type) {

	default:
		ethernet_response(mb, HRPN_RESP_STATUS_ERROR);
		break;
	}

	return;
}
