/*
 * Copyright 2022-2024 NXP
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
		"\nIndustrial CAN options:\n"
		"\t-r <id>        run CAN mode id:\n"
		"\t               0 - Multiple Nodes and Messages Tx+Rx on the imx8mp and the imx93}\n"
		"\t-n <node_type> acting as node 'A' or 'B' (default 'A')\n"
		"\t               0 - node 'A'\n"
		"\t               1 - node 'B'\n"
		"\t-o <protocol>  use CAN or CAN FD (default '0')\n"
		"\t               0 - use CAN\n"
		"\t               1 - use CAN FD\n"
		"\t-s             stop CAN\n"
	);
}

void ethernet_usage(void)
{
	printf(
		"\nIndustrial ethernet options:\n"
		"\t-a <mac_addr>  set hardware MAC address (default 00:bb:cc:dd:ee:14)\n"
		"\t-p <period_ns> set processing period in ns (default 100000)\n"
		"\t-r <id>        run ethernet mode id:\n"
		"\t               0 - genAVB/TSN stack with TSN application\n"
		"\t               1 - mcux-sdk API:\n"
		"\t                    imx8m{m,n}:    ENET on Zephyr and FreeRTOS\n"
		"\t                    imx8mp, imx93: ENET_QoS on Zephyr\n"
		"\t               2 - mcux-sdk API with PHY loopback mode:\n"
		"\t                    imx8mp, imx93: ENET_QoS on Zephyr\n"
		"\t-m <app_mode>  for genAVB/TSN: app mode (default 'NETWORK_ONLY', if not specified)\n"
		"\t               0 - mode is 'MOTOR_NETWORK'\n"
		"\t               2 - mode is 'NETWORK_ONLY'\n"
		"\t-i <role>      for genAVB/TSN: endpoint role (default 'controller', if not specified)\n"
		"\t               0 - role is 'IO device 0'\n"
		"\t               1 - role is 'IO device 1'\n"
		"\t-n <n_io_dev>  for NETWORK_ONLY and MOTOR_NETWORK app modes: number of connected io_devices (default is '1' if not specified. Max is '2')\n"
		"\t               	   max of two enpoints supported\n"
		"\t-c <ctrl_st>   for genAVB/TSN motor control: control strategy to be utilized (default is '0' if not specified)\n"
	    "\t               0 - SYNCHRONIZED\n"
		"\t               1 - FOLLOW\n"
		"\t               2 - HOLD_INDEX\n"
		"\t               3 - INTERLACED\n"
		"\t               4 - STOP\n"
		"\t               5 - IDENTIFY\n"
		"\t-s             stop ethernet\n"
	);
}

static int default_run(int fd, uint32_t type, uint32_t mode, uint32_t role, uint32_t period, uint32_t protocol, uint8_t *hw_addr, uint32_t num_io_devices, uint32_t control_strategy, uint32_t app_mode)
{
	struct hrpn_cmd_industrial_run run = {0,};
	struct hrpn_response resp;
	unsigned int len;

	run.type = type;
	run.mode = mode;
	run.role = role;
	run.period = period;
	run.protocol = protocol;
	run.num_io_devices = num_io_devices;
	run.control_strategy = control_strategy;
	run.app_mode = app_mode;

	if (hw_addr)
		memcpy(run.addr, hw_addr, sizeof(run.addr));

	len = sizeof(resp);

	return command(fd, &run, sizeof(run), HRPN_RESP_TYPE_INDUSTRIAL, &resp, &len, COMMAND_TIMEOUT);
}

static int default_stop(int fd, uint32_t type)
{
	struct hrpn_cmd_industrial_stop stop;
	struct hrpn_response resp;
	unsigned int len;

	stop.type = type;

	len = sizeof(resp);

	return command(fd, &stop, sizeof(stop), HRPN_RESP_TYPE_INDUSTRIAL, &resp, &len, COMMAND_TIMEOUT);
}

static int can_run(int fd, uint32_t mode, uint32_t role, uint32_t protocol)
{
	return default_run(fd, HRPN_CMD_TYPE_CAN_RUN, mode, role, 0, protocol, NULL, 0, 0, 0);
}

static int can_stop(int fd)
{
	return default_stop(fd, HRPN_CMD_TYPE_CAN_STOP);
}

static int ethernet_run(int fd, uint32_t mode, uint32_t role, uint32_t period, uint8_t *mac_addr, uint32_t num_io_devices, uint32_t control_strategy, uint32_t app_mode)
{
	return default_run(fd, HRPN_CMD_TYPE_ETHERNET_RUN, mode, role, period, 0, mac_addr, num_io_devices, control_strategy, app_mode);
}

static int ethernet_stop(int fd)
{
	return default_stop(fd, HRPN_CMD_TYPE_ETHERNET_STOP);
}

static int industrial_main(int option, char *optarg, int fd,
	int (*stop)(int))
{
	int rc = 0;

	switch (option) {
	case 's':
		rc = stop(fd);
		break;

	default:
		common_main(option, optarg);
		break;
	}

	return rc;
}

int can_main(int argc, char *argv[], int fd)
{
	unsigned int mode;
	unsigned int role = 0;
	unsigned int protocol = HRPN_PROTOCOL_CAN;
	int option;
	int rc = 0;
	bool is_run_cmd = false;

	while ((option = getopt(argc, argv, "r:sn:o:v")) != -1) {
		switch (option) {
		case 'r':
			if (strtoul_check(optarg, NULL, 0, &mode) < 0) {
				printf("Invalid mode\n");
			}
			is_run_cmd = true;
			break;

		case 'o':
			if (strtoul_check(optarg, NULL, 16, &protocol) < 0) {
				printf("Invalid protocol\n");
			}
			break;

		case 'n':
			if (strtoul_check(optarg, NULL, 16, &role) < 0) {
				printf("Invalid role\n");
			}
			break;

		default:
			rc = industrial_main(option, optarg, fd, can_stop);
			break;
		}
	}
	/* Run the case after we get all parameters */
	if (is_run_cmd)
		rc = can_run(fd, mode, role, protocol);
out:
	return rc;
}

int ethernet_main(int argc, char *argv[], int fd)
{
	unsigned int mode;
	unsigned int app_mode = DEFAULT_MODE;
	unsigned int role = DEFAULT_ROLE;
	unsigned int period = DEFAULT_PERIOD;
	uint8_t mac_addr[6] = MAC_ADDRESS_DEFAULT;
	unsigned int num_io_devices = DEFAULT_NUM_IO_DEV;
	unsigned int control_strategy = DEFAULT_CONTROL_STRATEGY;
	int option;
	int rc = 0;
	bool is_run_cmd = false;

	while ((option = getopt(argc, argv, "a:r:p:si:vn:c:m:")) != -1) {
		switch (option) {
		case 'a':
			if (read_mac_address(optarg, mac_addr) < 0) {
				printf("Invalid MAC address\n");
				rc = -1;
				goto out;
			}
			break;
		case 'p':
			if (strtoul_check(optarg, NULL, 0, &period) < 0) {
				printf("Invalid period\n");
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
				rc = -1;
				goto out;
			}
			role += 1; /* IO_DEVICE_0 offset */
			break;
		case 'n':
			if (strtoul_check(optarg, NULL, 0, &num_io_devices) < 0) {
				printf("Invalid number of io devices\n");
				rc = -1;
				goto out;
			}
			break;
		case 'c':
			if (strtoul_check(optarg, NULL, 0, &control_strategy) < 0) {
				printf("Invalid control strategy\n");
				rc = -1;
				goto out;
			}
			break;
		case 'm':
			if (strtoul_check(optarg, NULL, 0, &app_mode) < 0) {
				printf("Invalid mode\n");
				rc = -1;
				goto out;
			}
			break;
		default:
			rc = industrial_main(option, optarg, fd, ethernet_stop);
			break;
		}
	}
	/* Run the use case after we get all parameters */
	if (is_run_cmd)
		rc = ethernet_run(fd, mode, role, period, mac_addr, num_io_devices, control_strategy, app_mode);
out:
	return rc;
}
