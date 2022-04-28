/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "ethernet.h"
#include "hlog.h"
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

static void ethernet_set_mac_address(struct ethernet_ctx *ctx, uint8_t addr[6])
{
	uint8_t *mac = ctx->hw_addr;

	memcpy(mac, addr, sizeof(ctx->hw_addr));

	log_info("%02x:%02x:%02x:%02x:%02x:%02x\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void ethernet_ctrl(struct hrpn_cmd_ethernet *cmd, unsigned int len,
		   struct mailbox *mb, void *priv)
{
	struct ethernet_ctx *ctx = priv;

	switch (cmd->u.common.type) {
	case HRPN_CMD_TYPE_ETHERNET_SET_MAC_ADDR:
		if (!ctx || len != sizeof(struct hrpn_cmd_ethernet_set_mac_addr))
			goto err;

		ethernet_set_mac_address(ctx, cmd->u.set_mac_addr.mac.address);

		ethernet_response(mb, HRPN_RESP_STATUS_SUCCESS);

		break;

	default:
		ethernet_response(mb, HRPN_RESP_STATUS_ERROR);
		break;
	}

	return;

err:
	ethernet_response(mb, HRPN_RESP_STATUS_ERROR);
}
