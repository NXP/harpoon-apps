/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "hrpn_ctrl.h"
#include "common.h"

void can_usage(void)
{
	/* TODO */
}

void ethernet_usage(void)
{
	printf(
		"\nIndustrial ethernet options:\n"
		"\t-r             run ethernet\n"
		"\t-s             stop ethernet\n"
	);
}

static int default_run(struct mailbox *m, uint32_t type, uint32_t mode)
{
	struct hrpn_cmd_industrial_run run;
	struct hrpn_response resp;
	unsigned int len;

	run.type = type;
	run.mode = mode;

	len = sizeof(resp);

	return command(m, &run, sizeof(run), HRPN_RESP_TYPE_INDUSTRIAL, &resp, &len, COMMAND_TIMEOUT);
}

static int default_stop(struct mailbox *m, uint32_t type)
{
	struct hrpn_cmd_industrial_stop stop;
	struct hrpn_response resp;
	unsigned int len;

	stop.type = type;

	len = sizeof(resp);

	return command(m, &stop, sizeof(stop), HRPN_RESP_TYPE_INDUSTRIAL, &resp, &len, COMMAND_TIMEOUT);
}

static int can_run(struct mailbox *m)
{
	return default_run(m, HRPN_CMD_TYPE_CAN_RUN, 0);
}

static int can_stop(struct mailbox *m)
{
	return default_stop(m, HRPN_CMD_TYPE_CAN_STOP);
}

static int ethernet_run(struct mailbox *m)
{
	return default_run(m, HRPN_CMD_TYPE_ETHERNET_RUN, 0);
}

static int ethernet_stop(struct mailbox *m)
{
	return default_stop(m, HRPN_CMD_TYPE_ETHERNET_STOP);
}

static int industrial_main(int option, char *optarg, struct mailbox *m,
	int (*run)(struct mailbox *), int (*stop)(struct mailbox *))
{
	int rc = 0;

	switch (option) {
	case 'r':
		rc = run(m);
		break;

	case 's':
		rc = stop(m);
		break;

	default:
		common_main(option, optarg);
		break;
	}

	return rc;
}

int can_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	int rc = 0;

	while ((option = getopt(argc, argv, "rsv")) != -1) {
		switch (option) {
		default:
			rc = industrial_main(option, optarg, m, can_run, can_stop);
			break;
		}
	}
out:
	return rc;
}

int ethernet_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	int rc = 0;

	while ((option = getopt(argc, argv, "rsv")) != -1) {
		switch (option) {
		default:
			rc = industrial_main(option, optarg, m, ethernet_run, ethernet_stop);
			break;
		}
	}
out:
	return rc;
}
