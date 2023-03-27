/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#include <stdbool.h>

struct mailbox {
	bool dir;
	unsigned int last_cmd;
	unsigned int last_resp;
	void *cmd;
	void *resp;
	void *transport;
};

int mailbox_cmd_send(struct mailbox *mbox, void *data, unsigned int len);
int mailbox_cmd_recv(struct mailbox *mbox, void *data, unsigned int *len);
int mailbox_resp_send(struct mailbox *mbox, void *data, unsigned int len);
int mailbox_resp_recv(struct mailbox *mbox, void *data, unsigned int *len);

int mailbox_init(struct mailbox *mbox, void *cmd, void *resp, bool dir, void *tp);

#endif /* _MAILBOX_H_ */
