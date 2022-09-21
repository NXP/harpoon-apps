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

void audio_pipeline_usage(void)
{
	printf(
		"\nAudio pipeline options:\n"
		"\t-a <pipeline_id>  audio pipeline id (default 0)\n"
		"\t-d                audio pipeline dump\n"
	);
}

void audio_element_routing_usage(void)
{
	printf(
		"\nRouting audio element options:\n"
		"\t-a <pipeline_id>  audio pipeline id (default 0)\n"
		"\t-c                connect routing input/output\n"
		"\t-d                disconnect routing input/output\n"
		"\t-e <element_id>   routing element id (default 0)\n"
		"\t-i <input_id>     routing element input  (default 0)\n"
		"\t-o <output_id>    routing element output (default 0)\n"
	);
}

void audio_element_usage(void)
{
	printf(
		"\nAudio element options:\n"
		"\t-a <pipeline_id>  audio pipeline id (default 0)\n"
		"\t-d                audio element dump\n"
		"\t-e <element_id>   audio element id (default 0)\n"
		"\t-t <element_type> audio element type (default 0):\n"
		"\t                  0 - dtmf source\n"
		"\t                  1 - routing\n"
		"\t                  2 - sai sink\n"
		"\t                  3 - sai source\n"
		"\t                  4 - sine source\n"
		"\t                  5 - avtp source\n"
	);
}

static int audio_element_routing_connect(struct mailbox *m, unsigned int pipeline_id, unsigned int element_id, unsigned int output, unsigned int input)
{
	struct hrpn_cmd_audio_element_routing_connect connect;
	struct hrpn_resp_audio_element_routing resp;
	unsigned int len;

	connect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT;
	connect.pipeline.id = pipeline_id;
	connect.element.type = 1;
	connect.element.id = element_id;
	connect.output = output;
	connect.input = input;
	len = sizeof(resp);

	return command(m, &connect, sizeof(connect), HRPN_RESP_TYPE_AUDIO_ELEMENT_ROUTING, &resp, &len, COMMAND_TIMEOUT);
}

static int audio_element_routing_disconnect(struct mailbox *m, unsigned int pipeline_id, unsigned int element_id, unsigned int output)
{
	struct hrpn_cmd_audio_element_routing_disconnect disconnect;
	struct hrpn_resp_audio_element_routing resp;
	unsigned int len;

	disconnect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT;
	disconnect.pipeline.id = pipeline_id;
	disconnect.element.type = 1;
	disconnect.element.id = element_id;
	disconnect.output = output;
	len = sizeof(resp);

	return command(m, &disconnect, sizeof(disconnect), HRPN_RESP_TYPE_AUDIO_ELEMENT_ROUTING, &resp, &len, COMMAND_TIMEOUT);
}

int audio_element_routing_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	unsigned int pipeline_id = 0;
	unsigned int element_id = 0;
	unsigned int output = 0;
	unsigned int input = 0;
	int rc = 0;

	while ((option = getopt(argc, argv, "a:cde:i:o:v")) != -1) {
		switch (option) {
		case 'a':
			if (strtoul_check(optarg, NULL, 0, &pipeline_id) < 0) {
				printf("Invalid pipeline id\n");
				rc = -1;
				goto out;
			}

			break;

		case 'c':
			rc = audio_element_routing_connect(m, pipeline_id, element_id, output, input);

			break;

		case 'd':
			rc = audio_element_routing_disconnect(m, pipeline_id, element_id, output);

			break;

		case 'e':
			if (strtoul_check(optarg, NULL, 0, &element_id) < 0) {
				printf("Invalid element id\n");
				rc = -1;
				goto out;
			}

			break;

		case 'i':
			if (strtoul_check(optarg, NULL, 0, &input) < 0) {
				printf("Invalid element input\n");
				rc = -1;
				goto out;
			}

			break;

		case 'o':
			if (strtoul_check(optarg, NULL, 0, &output) < 0) {
				printf("Invalid element output\n");
				rc = -1;
				goto out;
			}

			break;

		default:
			common_main(option, optarg);
			break;
		}
	}

out:
	return rc;
}

static int audio_pipeline_element_dump(struct mailbox *m, unsigned int pipeline_id, unsigned int element_type, unsigned int element_id)
{
	struct hrpn_cmd_audio_element_dump dump;
	struct hrpn_resp_audio_element resp;
	unsigned int len;

	dump.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_DUMP;
	dump.pipeline.id = pipeline_id;
	dump.element.type = element_type;
	dump.element.id = element_id;
	len = sizeof(resp);

	return command(m, &dump, sizeof(dump), HRPN_RESP_TYPE_AUDIO_ELEMENT, &resp, &len, COMMAND_TIMEOUT);
}

int audio_element_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	unsigned int pipeline_id = 0;
	unsigned int element_type = 0;
	unsigned int element_id = 0;
	int rc = 0;

	while ((option = getopt(argc, argv, "a:de:t:v")) != -1) {
		switch (option) {
		case 'a':
			if (strtoul_check(optarg, NULL, 0, &pipeline_id) < 0) {
				printf("Invalid pipeline id\n");
				rc = -1;
				goto out;
			}

			break;

		case 'd':
			audio_pipeline_element_dump(m, pipeline_id, element_type, element_id);

			break;

		case 'e':
			if (strtoul_check(optarg, NULL, 0, &element_id) < 0) {
				printf("Invalid element id\n");
				rc = -1;
				goto out;
			}

			break;

		case 't':
			if (strtoul_check(optarg, NULL, 0, &element_type) < 0) {
				printf("Invalid element type\n");
				rc = -1;
				goto out;
			}

			break;

		default:
			common_main(option, optarg);
			break;
		}
	}

out:
	return rc;
}

static int audio_pipeline_dump(struct mailbox *m, unsigned int pipeline_id)
{
	struct hrpn_cmd_audio_pipeline_dump dump;
	struct hrpn_resp_audio_pipeline resp;
	unsigned int len;

	dump.type = HRPN_CMD_TYPE_AUDIO_PIPELINE_DUMP;
	dump.pipeline.id = pipeline_id;
	len = sizeof(resp);

	return command(m, &dump, sizeof(dump), HRPN_RESP_TYPE_AUDIO_PIPELINE, &resp, &len, COMMAND_TIMEOUT);
}

int audio_pipeline_main(int argc, char *argv[], struct mailbox *m)
{
	int option;
	unsigned int pipeline_id = 0;
	int rc = 0;

	while ((option = getopt(argc, argv, "a:dv")) != -1) {
		switch (option) {
		case 'a':
			if (strtoul_check(optarg, NULL, 0, &pipeline_id) < 0) {
				printf("Invalid pipeline id\n");
				rc = -1;
				goto out;
			}

			break;

		case 'd':
			audio_pipeline_dump(m, pipeline_id);

			break;

		default:
			common_main(option, optarg);
			break;
		}
	}

out:
	return rc;
}
