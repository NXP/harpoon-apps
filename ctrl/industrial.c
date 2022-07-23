/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "hrpn_ctrl.h"
#include "common.h"

#define MAC_ADDRESS_DEFAULT	{0x00, 0xBB, 0xCC, 0xDD, 0xEE, 0x14}

void can_usage(void)
{
	printf(
		"\nIndustrial CAN options:\n"
		"\t-r <id>        run CAN mode id:\n"
		"\t               0 - loopback\n"
		"\t               1 - interrupt\n"
		"\t               2 - pingpong\n"
		"\t-n <node_type> acting as node 'A' or 'B' (default 'A')\n"
		"\t               0 - node 'A'\n"
		"\t               1 - node 'B'\n"
		"\t-s             stop CAN\n"
	);
}

void ethernet_usage(void)
{
	printf(
		"\nIndustrial ethernet options:\n"
		"\t-a <mac_addr>  set hardware MAC address (default 91:e0:f0:00:fe:70)\n"
		"\t-r <id>        run ethernet mode id:\n"
		"\t               0 - genAVB/TSN stack\n"
		"\t               1 - mcux-sdk API (imx8m{m,n} ENET)\n"
		"\t-i <role>      for genAVB/TSN: endpoint role (default 'controller', if not specified)\n"
		"\t               0 - role is 'IO device 0'\n"
		"\t               1 - role is 'IO device 1'\n"
		"\t-s             stop ethernet\n"
	);
}

static int default_run(struct mailbox *m, uint32_t type, uint32_t mode, uint32_t role, uint8_t *hw_addr)
{
	struct hrpn_cmd_industrial_run run = {0,};
	struct hrpn_response resp;
	unsigned int len;

	run.type = type;
	run.mode = mode;
	run.role = role;

	if (hw_addr)
		memcpy(run.addr, hw_addr, sizeof(run.addr));

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

static int can_run(struct mailbox *m, uint32_t mode, uint32_t role)
{
	return default_run(m, HRPN_CMD_TYPE_CAN_RUN, mode, role, NULL);
}

static int can_stop(struct mailbox *m)
{
	return default_stop(m, HRPN_CMD_TYPE_CAN_STOP);
}

static int ethernet_run(struct mailbox *m, uint32_t mode, uint32_t role, uint8_t *mac_addr)
{
	return default_run(m, HRPN_CMD_TYPE_ETHERNET_RUN, mode, role, mac_addr);
}

static int ethernet_stop(struct mailbox *m)
{
	return default_stop(m, HRPN_CMD_TYPE_ETHERNET_STOP);
}

static int read_mac_address(char *buf, uint8_t *mac)
{
#define NB_OCTETS	6
    int canary = 3 * NB_OCTETS - 1; /* octets + ":" separators */
    int rc;

    buf[canary] = '\0';
    rc = sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    return (rc == NB_OCTETS) ? 0 : -1;
}

static int industrial_main(int option, char *optarg, struct mailbox *m,
	int (*stop)(struct mailbox *))
{
	int rc = 0;

	switch (option) {
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
	unsigned int mode;
	unsigned int role = 0;
	int option;
	int rc = 0;
	bool is_run_cmd = false;

	while ((option = getopt(argc, argv, "r:sn:v")) != -1) {
		switch (option) {
		case 'r':
			if (strtoul_check(optarg, NULL, 0, &mode) < 0) {
				printf("Invalid mode\n");
			}
			is_run_cmd = true;
			break;
		case 'n':
			if (strtoul_check(optarg, NULL, 16, &role) < 0) {
				printf("Invalid role\n");
			}
			break;

		default:
			rc = industrial_main(option, optarg, m, can_stop);
			break;
		}
	}
	/* Run the case after we get all parameters */
	if (is_run_cmd)
		rc = can_run(m, mode, role);
out:
	return rc;
}

int ethernet_main(int argc, char *argv[], struct mailbox *m)
{
	unsigned int mode;
	unsigned int role = 0;
	uint8_t mac_addr[6] = MAC_ADDRESS_DEFAULT;
	int option;
	int rc = 0;
	bool is_run_cmd = false;

	while ((option = getopt(argc, argv, "a:r:si:v")) != -1) {
		switch (option) {
		case 'a':
			if (read_mac_address(optarg, mac_addr) < 0) {
				printf("Invalid MAC address\n");
				rc = -1;
				goto out;
			}
			break;
		case 'r':
			if (strtoul_check(optarg, NULL, 0, &mode) < 0) {
				printf("Invalid mode\n");
			}
			is_run_cmd = true;
			break;
		case 'i':
			if (strtoul_check(optarg, NULL, 16, &role) < 0) {
				printf("Invalid role\n");
			}
			role += 1; /* IO_DEVICE_0 offset */
			break;

		default:
			rc = industrial_main(option, optarg, m, ethernet_stop);
			break;
		}
	}
	/* Run the use case after we get all parameters */
	if (is_run_cmd)
		rc = ethernet_run(m, mode, role, mac_addr);
out:
	return rc;
}
