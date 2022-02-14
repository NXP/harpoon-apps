/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "version.h"
#include "hrpn_ctrl.h"

#include "common.h"

extern const struct cmd_handler command_handler[5];

int command(struct mailbox *m, void *cmd, unsigned int cmd_len, unsigned int resp_type, void *resp, unsigned int *resp_len, unsigned int timeout_ms)
{
	int count = timeout_ms / 100;
	struct hrpn_response *r;
	int rc;

	rc = mailbox_cmd_send(m, cmd, cmd_len);
	if (!rc) {
		while (mailbox_resp_recv(m, resp, resp_len) < 0) {
			usleep(100000);
			count--;
			if (count < 0) {
				rc = -1;
				printf("command timeout\n");
				goto exit;
			}
		}

		r = resp;

		if (r->u.resp.type != resp_type) {
			rc = -1;
			printf("command response mismatch: %x\n", r->u.resp.type);
		} else if (r->u.resp.status != HRPN_RESP_STATUS_SUCCESS) {
			rc = -1;
			printf("command failed\n");
		} else {
			printf("command success\n");
		}
	} else {
		printf("command send error\n");
	}

exit:
	return rc;
}

int strtoul_check(const char *nptr, char **endptr, int base, unsigned int *val)
{
	errno = 0;

	*val = strtoul(nptr, endptr, base);
	if (errno)
		return -1;

	return 0;
}

void usage(void)
{
	int i;

	printf("\nUsage:\nharpoon_ctrl [");

	for (i = 0; i < sizeof(command_handler) / sizeof(struct cmd_handler) - 1; i++)
		printf("%s|", command_handler[i].name);

	printf( "%s] [options]\n", command_handler[i].name);

	printf( "\nOptions:\n");

	for (i = 0; i < sizeof(command_handler) / sizeof(struct cmd_handler); i++)
		command_handler[i].usage();

	printf( "\nCommon options:\n"
		"\t-v             print version\n");
}

void common_main(int option, char *optarg)
{
	switch (option) {
	case 'v':
		printf("Harpoon v%s\n", VERSION);
		break;

	default:
		usage();
		break;
	}
}
