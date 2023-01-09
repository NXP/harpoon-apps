/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _HRPN_CTRL_H_
#define _HRPN_CTRL_H_

#include <stdint.h>

#include "hrpn_ctrl_audio_pipeline.h"

enum {
	HRPN_CMD_TYPE_LATENCY_RUN = 0x0000,
	HRPN_CMD_TYPE_LATENCY_STOP,
	HRPN_RESP_TYPE_LATENCY = 0x0010,

	HRPN_CMD_TYPE_AUDIO_RUN = 0x0100,
	HRPN_CMD_TYPE_AUDIO_STOP,
	HRPN_RESP_TYPE_AUDIO = 0x0110,

	HRPN_CMD_TYPE_AUDIO_PIPELINE_DUMP = 0x200,
	HRPN_RESP_TYPE_AUDIO_PIPELINE = 0x2ff,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_DUMP = 0x300,
	HRPN_RESP_TYPE_AUDIO_ELEMENT = 0x3ff,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT = 0x400,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT = 0x401,
	HRPN_RESP_TYPE_AUDIO_ELEMENT_ROUTING = 0x44f,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ENABLE = 0x450,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_DISABLE = 0x451,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ID = 0x452,
	HRPN_RESP_TYPE_AUDIO_ELEMENT_PLL = 0x45f,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_CONNECT = 0x480,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_DISCONNECT = 0x481,
	HRPN_RESP_TYPE_AUDIO_ELEMENT_AVTP = 0x48f,

	HRPN_CMD_TYPE_INDUSTRIAL = 0x500,
	HRPN_CMD_TYPE_CAN_RUN = 0x580,
	HRPN_CMD_TYPE_CAN_STOP,
	HRPN_CMD_TYPE_ETHERNET_RUN = 0x600,
	HRPN_CMD_TYPE_ETHERNET_STOP,
	HRPN_RESP_TYPE_INDUSTRIAL = 0x6ff,
};

enum {
	HRPN_RESP_STATUS_SUCCESS = 0,
	HRPN_RESP_STATUS_ERROR = 1,
};

enum {
	HRPN_PROTOCOL_CAN = 0,
	HRPN_PROTOCOL_CAN_FD = 1,
};

struct hrpn_cmd_latency_run {
	uint32_t type;
	uint32_t id;
};

struct hrpn_cmd_latency_stop {
	uint32_t type;
};

struct hrpn_resp_latency {
	uint32_t type;
	uint32_t status;
};

/* Audio application commands */
struct hrpn_cmd_audio_run {
	uint32_t type;
	uint32_t id;
	uint32_t frequency;
	uint32_t period;
};

struct hrpn_cmd_audio_stop {
	uint32_t type;
};

struct hrpn_resp_audio {
	uint32_t type;
	uint32_t status;
};

/* Industrial application commands */
struct hrpn_cmd_industrial_run {
	uint32_t type;
	uint32_t mode;
	uint32_t role;
	uint32_t protocol;
	uint8_t addr[6];
};

struct hrpn_cmd_industrial_stop {
	uint32_t type;
};

struct hrpn_resp_industrial {
	uint32_t type;
	uint32_t status;
};

struct hrpn_cmd_ethernet_addr {
	uint8_t address[6];
};

struct hrpn_cmd_ethernet_common {
	uint32_t type;		/* command type */
};

struct hrpn_cmd_ethernet_set_mac_addr {
	uint32_t type;		/* command type */
	struct hrpn_cmd_ethernet_addr mac;
};

struct hrpn_cmd_ethernet {
	union {
    struct hrpn_cmd_ethernet_common common;
    struct hrpn_cmd_ethernet_set_mac_addr set_mac_addr;
	} u;
};

struct hrpn_cmd {
	uint32_t type;
};

struct hrpn_resp {
	uint32_t type;
	uint32_t status;
};

struct hrpn_command {
	union {
		struct hrpn_cmd cmd;
		struct hrpn_cmd_latency_run latency_run;
		struct hrpn_cmd_latency_stop latency_stop;
		struct hrpn_cmd_audio_run audio_run;
		struct hrpn_cmd_audio_stop audio_stop;
		struct hrpn_cmd_audio_pipeline audio_pipeline;
		struct hrpn_cmd_industrial_run industrial_run;
		struct hrpn_cmd_industrial_stop industrial_stop;
		struct hrpn_cmd_ethernet ethernet;
	} u;
};

struct hrpn_response {
	union {
		struct hrpn_resp resp;
		struct hrpn_resp_latency latency;
		struct hrpn_resp_audio audio;
		struct hrpn_resp_industrial industrial;
	} u;
};

#if (CONFIG_GENAVB_ENABLE == 1)
#include "genavb/control.h"
#endif

#endif /* _HRPN_CTRL_H_ */
