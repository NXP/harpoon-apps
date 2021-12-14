/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "ivshmem.h"
#include "mailbox.h"
#include "version.h"
#include "hrpn_ctrl.h"

#define COMMAND_TIMEOUT	1000	/* 1 sec */

static void usage(void)
{
	printf("\nUsage:\nharpoon_ctrl [latency|audio] [options]\n");
	printf("\nOptions:\n"
		"\nLatency options:\n"
		"\t-r <id>        run latency test case id\n"
		"\t-s             stop running test case\n"
		"\nAudio options:\n"
		"\n-f <frequency> audio clock frequency (in Hz)\n"
		"\n               Supporting 44100, 48000, 88200, 176400, 96000, 192000 Hz\n"
		"\n               Will use default frequency 44100Hz if not specified\n"
		"\n-p <samples>   audio processing period (in samples)\n"
		"\n               Supporting 4, 8, 16, 32, 64 samples\n"
		"\n               Will use default period 64 samples if not specified\n"
		"\n-r <id>        run audio mode id:\n"
		"\n           0 - dtmf playback\n"
		"\n           1 - music playback\n"
		"\n           2 - sine wave playback\n"
		"\n           3 - playback & recording (loopback)\n"
		"\n-s             stop running audio mode\n"
		"\nCommon options:\n"
		"\t-v             print version\n");
}

static int command(struct mailbox *m, void *cmd, unsigned int cmd_len, unsigned int resp_type, void *resp, unsigned int *resp_len, unsigned int timeout_ms)
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

static int audio_run(struct mailbox *m, unsigned int id, unsigned int frequency, unsigned int period)
{
	struct hrpn_cmd_audio_run run;
	struct hrpn_response resp;
	unsigned int len;

	run.type = HRPN_CMD_TYPE_AUDIO_RUN;
	run.id = id;
	run.frequency = frequency;
	run.period = period;

	len = sizeof(resp);

	return command(m, &run, sizeof(run), HRPN_RESP_TYPE_AUDIO, &resp, &len, COMMAND_TIMEOUT);
}

static int audio_stop(struct mailbox *m)
{
	struct hrpn_cmd_audio_stop stop;
	struct hrpn_response resp;
	unsigned int len;

	stop.type = HRPN_CMD_TYPE_AUDIO_STOP;

	len = sizeof(resp);

	return command(m, &stop, sizeof(stop), HRPN_RESP_TYPE_AUDIO, &resp, &len, COMMAND_TIMEOUT);
}

static int audio_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	unsigned int id;
	int rc = 0;
	unsigned int frequency = 0;
	unsigned int period = 0;
	bool is_run_cmd = false;

	while ((option = getopt(argc, argv, "f:p:r:sv")) != -1) {
		switch (option) {
		case 'f':
			frequency = strtoul(optarg, NULL, 0);
			break;

		case 'p':
			period = strtoul(optarg, NULL, 0);
			break;

		case 'r':
			id = strtoul(optarg, NULL, 0);
			is_run_cmd = true;
			break;

		case 's':
			rc = audio_stop(m);
			break;

		case 'v':
			printf("Harpoon v.%s\n", VERSION);
			break;

		default:
			usage();
			break;
		}
	}
	/* Run the case after we get all parameters */
	if (is_run_cmd)
		rc = audio_run(m, id, frequency, period);

	return rc;
}

static int latency_run(struct mailbox *m, unsigned int id)
{
	struct hrpn_cmd_latency_run run;
	struct hrpn_response resp;
	unsigned int len;

	run.type = HRPN_CMD_TYPE_LATENCY_RUN;
	run.id = id;

	len = sizeof(resp);

	return command(m, &run, sizeof(run), HRPN_RESP_TYPE_LATENCY, &resp, &len, COMMAND_TIMEOUT);
}

static int latency_stop(struct mailbox *m)
{
	struct hrpn_cmd_latency_stop stop;
	struct hrpn_response resp;
	unsigned int len;

	stop.type = HRPN_CMD_TYPE_LATENCY_STOP;

	len = sizeof(resp);

	return command(m, &stop, sizeof(stop), HRPN_RESP_TYPE_LATENCY, &resp, &len, COMMAND_TIMEOUT);
}

static int latency_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	unsigned int id;
	int rc = 0;

	while ((option = getopt(argc, argv, "r:sv")) != -1) {
		/* common options */
		switch (option) {
		case 'r':
			id = strtoul(optarg, NULL, 0);

			rc = latency_run(m, id);
			break;

		case 's':
			rc = latency_stop(m);
			break;

		case 'v':
			printf("Harpoon v.%s\n", VERSION);
			break;
		default:
			usage();
			break;
		}
	}

	return rc;
}

int main(int argc, char *argv[])
{
	struct ivshmem mem;
	struct mailbox m;
	unsigned int uio_id = 0;
	int rc;

	if (argc < 2) {
		usage();
		goto err;
	}

	if (ivshmem_init(&mem, uio_id) < 0)
		goto err_ivshmem;

	if (mailbox_init(&m, mem.out, mem.in + 2 * 4096, true) < 0)
		goto err_mailbox;

	if (!strcmp("audio", argv[1])) {
		rc = audio_main(argc - 1, argv + 1, &m);
	} else if (!strcmp("latency", argv[1]))  {
		rc = latency_main(argc - 1, argv + 1, &m);
	} else {
		usage();
		goto exit;
	}

exit:
	ivshmem_exit(&mem);

	return rc;

err_mailbox:
	ivshmem_exit(&mem);

err_ivshmem:
err:
	return -1;
}
