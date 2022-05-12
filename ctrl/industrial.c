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

void can_usage(void)
{
	printf(
		"\t-r <id>        run CAN mode id:\n"
		"\t               0 - loopback\n"
		"\t               1 - interrupt\n"
		"\t               2 - pingpong\n"
		"\t-s             stop CAN\n"
	);
}

void ethernet_usage(void)
{
	printf(
		"\nIndustrial ethernet options:\n"
		"\t-a <mac_addr>  set hardware MAC address (default 91:e0:f0:00:fe:70)\n"
		"\t-r <id>        run ethernet mode id:\n"
		"\t               0 - genAVB/TSN stack (imx8mp ENET_QOS)\n"
		"\t               1 - mcux-sdk API (im8m{m,n} ENET)\n"
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

static int can_run(struct mailbox *m, uint32_t mode)
{
	return default_run(m, HRPN_CMD_TYPE_CAN_RUN, mode);
}

static int can_stop(struct mailbox *m)
{
	return default_stop(m, HRPN_CMD_TYPE_CAN_STOP);
}

static int ethernet_run(struct mailbox *m, uint32_t mode)
{
	return default_run(m, HRPN_CMD_TYPE_ETHERNET_RUN, mode);
}

static int ethernet_stop(struct mailbox *m)
{
	return default_stop(m, HRPN_CMD_TYPE_ETHERNET_STOP);
}

static int ethernet_set_mac_address(struct mailbox *m, uint8_t mac_addr[6])
{
	struct hrpn_cmd_ethernet_set_mac_addr set_mac_addr;
	struct hrpn_resp_industrial resp;
	unsigned int len;

	set_mac_addr.type = HRPN_CMD_TYPE_ETHERNET_SET_MAC_ADDR;
	memcpy(set_mac_addr.mac.address, mac_addr, sizeof(set_mac_addr.mac.address));
	len = sizeof(resp);

	return command(m, &set_mac_addr, sizeof(set_mac_addr), HRPN_RESP_TYPE_INDUSTRIAL, &resp, &len, COMMAND_TIMEOUT);
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
	int option;
	int rc = 0;

	while ((option = getopt(argc, argv, "r:sv")) != -1) {
		switch (option) {
		case 'r':
			if (strtoul_check(optarg, NULL, 0, &mode) < 0) {
				printf("Invalid mode\n");
			}
			rc = can_run(m, mode);
			break;

		default:
			rc = industrial_main(option, optarg, m, can_stop);
			break;
		}
	}
out:
	return rc;
}

int ethernet_main(int argc, char *argv[], struct mailbox *m)
{
	unsigned int mode;
	uint8_t mac_addr[6];
	int option;
	int rc = 0;

	while ((option = getopt(argc, argv, "a:r:sv")) != -1) {
		switch (option) {
		case 'a':
			if (read_mac_address(optarg, mac_addr) < 0) {
				printf("Invalid MAC address\n");
				rc = -1;
				goto out;
			}
			ethernet_set_mac_address(m, mac_addr);
			break;

		case 'r':
			if (strtoul_check(optarg, NULL, 0, &mode) < 0) {
				printf("Invalid mode\n");
			}
			rc = ethernet_run(m, mode);
			break;

		default:
			rc = industrial_main(option, optarg, m, ethernet_stop);
			break;
		}
	}
out:
	return rc;
}
