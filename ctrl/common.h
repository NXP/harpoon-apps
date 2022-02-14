/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#define COMMAND_TIMEOUT	5000	/* 5 sec */

#include "mailbox.h"

struct cmd_handler {
	const char *name;
	int (* main)(int argc, char *argv[], struct mailbox *m);
	void (* usage)(void);
};

int command(struct mailbox *m, void *cmd, unsigned int cmd_len, unsigned int resp_type, void *resp, unsigned int *resp_len, unsigned int timeout_ms);
int strtoul_check(const char *nptr, char **endptr, int base, unsigned int *val);
void usage(void);
void common_main(int option, char *optarg);

#endif /* _COMMON_H_ */
