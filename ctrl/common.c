/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "version.h"
#include "hrpn_ctrl.h"
#include "rpmsg.h"

#include "common.h"

extern const struct cmd_handler command_handler[7];

int command(int fd, void *cmd, unsigned int cmd_len, unsigned int resp_type, void *resp, unsigned int *resp_len, unsigned int timeout_ms)
{
	int count = timeout_ms / 100;
	struct hrpn_response *r;
	int rc;

	rc = rpmsg_send(fd, cmd, cmd_len);
	if (!rc) {
		while (rpmsg_recv(fd, resp, resp_len, 0) < 0) {
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

int read_mac_address(char *buf, uint8_t *mac)
{
#define NB_OCTETS	6
	int canary = 3 * NB_OCTETS - 1; /* octets + ":" separators */
	int rc;

	buf[canary] = '\0';
	rc = sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	return (rc == NB_OCTETS) ? 0 : -1;
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
