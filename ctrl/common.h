/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#define COMMAND_TIMEOUT	5000	/* 5 sec */
#define MAC_ADDRESS_DEFAULT	{0x00, 0xBB, 0xCC, 0xDD, 0xEE, 0x14}
#define DEFAULT_PERIOD 100000
#define DEFAULT_NUM_IO_DEV 1
#define DEFAULT_CONTROL_STRATEGY 0
#define DEFAULT_ROLE 0
#define DEFAULT_MODE 2

struct cmd_handler {
	const char *name;
	int (* main)(int argc, char *argv[], int fd);
	void (* usage)(void);
};

int command(int fd, void *cmd, unsigned int cmd_len, unsigned int resp_type, void *resp, unsigned int *resp_len, unsigned int timeout_ms);
int strtoul_check(const char *nptr, char **endptr, int base, unsigned int *val);
int read_mac_address(char *buf, uint8_t *mac);
void usage(void);
void common_main(int option, char *optarg);

#endif /* _COMMON_H_ */
