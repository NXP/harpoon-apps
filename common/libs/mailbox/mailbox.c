/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include "mailbox.h"


#define MAX_PAYLOAD	255

static inline void __DSB(void)
{
	__asm volatile ("dsb sy");
}

struct cmd {
	volatile uint32_t seq;
	uint32_t len;
	uint8_t data[MAX_PAYLOAD];
};

struct resp {
	volatile uint32_t seq;
	uint32_t len;
	uint8_t data[MAX_PAYLOAD];
};

/* Command Sender */
int mailbox_cmd_send(struct mailbox *mbox, void *data, unsigned int len)
{
	struct cmd *cmd = mbox->cmd;

	if (!mbox->dir)
		return -1;

	if (len > MAX_PAYLOAD)
		return -1;

	/* write new command */
	cmd->len = len;
	memcpy(cmd->data, data, len);

	mbox->last_cmd += 1;

	__DSB();

	cmd->seq = mbox->last_cmd;

	return 0;
}

int mailbox_resp_recv(struct mailbox *mbox, void *data, unsigned int *len)
{
	struct resp *r = mbox->resp;
	unsigned int resp_len;

	if (!mbox->dir)
		return -1;

	/* check if new response */
	if (r->seq == mbox->last_resp)
		return -1;

	mbox->last_resp = r->seq;

	__DSB();

	/* check if it matches command */
	if (r->seq != mbox->last_cmd)
		return -2;

	resp_len = r->len;

	if (resp_len <= *len)
		*len = resp_len;

	memcpy(data, r->data, *len);

	return 0;
}

/* Command Receiver */
int mailbox_cmd_recv(struct mailbox *mbox, void *data, unsigned int *len)
{
	struct cmd *c = mbox->cmd;
	unsigned int cmd_len;

	if (mbox->dir)
		return -1;

	/* check if new command */
	if (c->seq == mbox->last_cmd)
		return -1;

	mbox->last_cmd = c->seq;

	__DSB();

	cmd_len = c->len;

	if (cmd_len <= *len)
		*len = cmd_len;

	memcpy(data, c->data, *len);

	return 0;
}

int mailbox_resp_send(struct mailbox *mbox, void *data, unsigned int len)
{
	struct resp *r = mbox->resp;

	if (mbox->dir)
		return -1;

	if (len > MAX_PAYLOAD)
		return -1;

	/* write new response */
	r->len = len;
	memcpy(r->data, data, len);

	__DSB();

	r->seq = mbox->last_cmd;

	return 0;
}

int mailbox_init(struct mailbox *mbox, void *cmd, void *resp, bool dir)
{
	struct resp *r;
	struct cmd *c;

	mbox->dir = dir;
	mbox->cmd = cmd;
	mbox->resp = resp;

	c = cmd;
	r = resp;

	if (dir) {
		/* command sender */
		/* just initialize with current state */
		mbox->last_cmd = c->seq;

		/* make sure, next command doesn't match current response */
		if ((mbox->last_cmd + 1) == r->seq)
			mbox->last_cmd++;

		mbox->last_resp = r->seq;
	} else {
		/* command receiver */
		/* always ignore first pending command */
		mbox->last_cmd = c->seq;

		/* always update response, making sure it changes and
		 * doesn't match current command */
		mbox->last_resp = r->seq + 1;

		if (mbox->last_resp == mbox->last_cmd)
			mbox->last_resp++;

		r->seq = mbox->last_resp;
	}

	return 0;
}
