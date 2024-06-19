/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "hrpn_ctrl.h"
#include "rpmsg.h"
#include "common.h"

int audio_element_routing_main(int argc, char *argv[], int fd);
int audio_element_main(int argc, char *argv[], int fd);
int audio_pipeline_main(int argc, char *argv[], int fd);
void audio_pipeline_usage(void);
void audio_element_routing_usage(void);
void audio_element_usage(void);

int can_main(int argc, char *argv[], int fd);
int ethernet_main(int argc, char *argv[], int fd);
void can_usage(void);
void ethernet_usage(void);

static void latency_usage(void)
{
	printf(
		"\nLatency options:\n"
		"\t-r <id>        run latency test case id\n"
		"\t-s             stop running test case\n"
	);
}

static void audio_usage(void)
{
	printf(
		"\nAudio options:\n"
		"\t-f <frequency> audio clock frequency (in Hz)\n"
		"\t               imx8m{n,m,p}: supporting 44100, 48000, 88200, 96000, 176400, 192000 Hz\n"
		"\t               imx93: supporting 48000, 96000 Hz\n"
		"\t                      supporting 48000, 96000, 192000 Hz using MX93AUD-HAT\n"
		"\t               Will use default frequency 48000Hz if not specified\n"
		"\t-p <frames>    audio processing period (in frames)\n"
		"\t               Supporting 2, 4, 8, 16, 32 frames\n"
		"\t               Supporting 2, 4, 8 frames on MX93AUD-HAT\n"
		"\t               Will use default period 8 frames if not specified\n"
		"\t-a <mac_addr>  set hardware MAC address (default 00:bb:cc:dd:ee:14)\n"
		"\t-r <id>        run audio mode id:\n"
		"\t               0 - dtmf playback\n"
		"\t               1 - sine wave playback\n"
		"\t               2 - playback & recording (loopback)\n"
		"\t               3 - audio pipeline\n"
		"\t               4 - AVB audio pipeline\n"
		"\t               5 - SMP audio pipeline on imx8m{n,m,p}\n"
		"\t               6 - AVB audio pipeline (with MCR support) only on i.mx8mp\n"
		"\t               7 - SMP + AVB audio pipeline on imx8m{n,m,p}\n"
		"\t               8 - SMP + AVB audio pipeline (with MCR support) only on i.mx8mp\n"
		"\t-H             select the MX93AUD-HAT extension audio board. Only on i.mx93\n"
		"\t-s             stop running audio mode\n"
	);
}

static int audio_run(int fd, unsigned int id, unsigned int frequency, unsigned int period, uint8_t *hw_addr, bool use_audio_hat)
{
	struct hrpn_cmd_audio_run run = {0,};
	struct hrpn_response resp;
	unsigned int len;

	run.type = HRPN_CMD_TYPE_AUDIO_RUN;
	run.id = id;
	run.frequency = frequency;
	run.period = period;
	run.use_audio_hat = use_audio_hat;

	memcpy(run.addr, hw_addr, sizeof(run.addr));

	len = sizeof(resp);

	return command(fd, &run, sizeof(run), HRPN_RESP_TYPE_AUDIO, &resp, &len, COMMAND_TIMEOUT);
}

static int audio_stop(int fd)
{
	struct hrpn_cmd_audio_stop stop;
	struct hrpn_response resp;
	unsigned int len;

	stop.type = HRPN_CMD_TYPE_AUDIO_STOP;

	len = sizeof(resp);

	return command(fd, &stop, sizeof(stop), HRPN_RESP_TYPE_AUDIO, &resp, &len, COMMAND_TIMEOUT);
}

static int audio_main(int argc, char *argv[], int fd)
{
	int option;
	unsigned int id;
	int rc = 0;
	unsigned int frequency = 0;
	unsigned int period = 0;
	bool use_audio_hat = false;
	uint8_t mac_addr[6] = MAC_ADDRESS_DEFAULT;
	bool is_run_cmd = false;

	while ((option = getopt(argc, argv, "f:p:r:a:Hsv")) != -1) {
		switch (option) {
		case 'f':
			if (strtoul_check(optarg, NULL, 0, &frequency) < 0) {
				printf("Invalid frequency\n");
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

		case 'a':
			if (read_mac_address(optarg, mac_addr) < 0) {
				printf("Invalid MAC address\n");
				rc = -1;
				goto out;
			}

			break;

		case 'r':
			if (strtoul_check(optarg, NULL, 0, &id) < 0) {
				printf("Invalid id\n");
				rc = -1;
				goto out;
			}

			is_run_cmd = true;
			break;

		case 'H':
			use_audio_hat = true;
			break;

		case 's':
			rc = audio_stop(fd);
			if (system("/usr/share/harpoon/scripts/harpoon_configure.sh audio stop") != 0) {
				printf("configure script failed.\n");
				rc = -1;
				goto out;
			}

			break;

		default:
			common_main(option, optarg);
			break;
		}
	}

	if (use_audio_hat && is_run_cmd) {
		if (system("/usr/share/harpoon/scripts/harpoon_configure.sh audio start_audio_hat") != 0) {
			printf("configure script failed.\n");
			rc = -1;
			goto out;
		}
	} else if (is_run_cmd) {
		if (system("/usr/share/harpoon/scripts/harpoon_configure.sh audio start") != 0) {
			printf("configure script failed.\n");
			rc = -1;
			goto out;
		}
	}

	/* Run the case after we get all parameters */	
	if (is_run_cmd)
		rc = audio_run(fd, id, frequency, period, mac_addr, use_audio_hat);

out:
	return rc;
}

static int latency_run(int fd, unsigned int id)
{
	struct hrpn_cmd_latency_run run;
	struct hrpn_response resp;
	unsigned int len;

	run.type = HRPN_CMD_TYPE_LATENCY_RUN;
	run.id = id;

	len = sizeof(resp);

	return command(fd, &run, sizeof(run), HRPN_RESP_TYPE_LATENCY, &resp, &len, COMMAND_TIMEOUT);
}

static int latency_stop(int fd)
{
	struct hrpn_cmd_latency_stop stop;
	struct hrpn_response resp;
	unsigned int len;

	stop.type = HRPN_CMD_TYPE_LATENCY_STOP;

	len = sizeof(resp);

	return command(fd, &stop, sizeof(stop), HRPN_RESP_TYPE_LATENCY, &resp, &len, COMMAND_TIMEOUT);
}

static int latency_main(int argc, char *argv[], int fd)
{
	int option;
	unsigned int id;
	int rc = 0;

	while ((option = getopt(argc, argv, "r:sv")) != -1) {
		/* common options */
		switch (option) {
		case 'r':
			if (strtoul_check(optarg, NULL, 0, &id) < 0) {
				printf("Invalid id\n");
				rc = -1;
				goto out;
			}

			rc = latency_run(fd, id);

			break;

		case 's':
			rc = latency_stop(fd);
			break;

		default:
			common_main(option, optarg);
			break;
		}
	}

out:
	return rc;
}

const struct cmd_handler command_handler[] = {
	{ "audio", audio_main, audio_usage },
	{ "latency", latency_main, latency_usage },
	{ "pipeline", audio_pipeline_main, audio_pipeline_usage },
	{ "element", audio_element_main, audio_element_usage },
	{ "routing", audio_element_routing_main, audio_element_routing_usage },

	{ "can", can_main, can_usage },
	{ "ethernet", ethernet_main, ethernet_usage },
};

int main(int argc, char *argv[])
{
	int fd, i;
	int rc = 0;
	unsigned int dst = 30;

	if (argc < 2) {
		usage();
		goto err;
	}

	fd = rpmsg_init(dst);
	if (fd < 0)
		goto err_rpmsg;

	for (i = 0; i < sizeof(command_handler) / sizeof(struct cmd_handler); i++)
		if (!strcmp(command_handler[i].name, argv[1])) {
			rc = command_handler[i].main(argc - 1, argv + 1, fd);
			goto exit;
		}

	usage();

exit:
	rpmsg_deinit(fd);
	return rc;

err_rpmsg:
	printf("%s: err_rpmsg\n", __func__);

err:
	return -1;
}
