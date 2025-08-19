/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _HRPN_CTRL_H_
#define _HRPN_CTRL_H_

#include <stdint.h>
#include <stdbool.h>

#include "rtos_apps/audio/audio_ctrl.h"

enum {
	HRPN_CMD_TYPE_LATENCY_RUN = 0x0000,
	HRPN_CMD_TYPE_LATENCY_STOP,
	HRPN_RESP_TYPE_LATENCY = 0x0010,

	HRPN_CMD_TYPE_AUDIO_RUN = AUDIO_CMD_TYPE_RUN,
	HRPN_CMD_TYPE_AUDIO_STOP = AUDIO_CMD_TYPE_STOP,
	HRPN_RESP_TYPE_AUDIO = AUDIO_RESP_TYPE,

	HRPN_CMD_TYPE_AUDIO_PIPELINE_DUMP = AUDIO_CMD_TYPE_PIPELINE_DUMP,
	HRPN_RESP_TYPE_AUDIO_PIPELINE = AUDIO_RESP_TYPE_PIPELINE,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_DUMP = AUDIO_CMD_TYPE_ELEMENT_DUMP,
	HRPN_RESP_TYPE_AUDIO_ELEMENT = AUDIO_RESP_TYPE_ELEMENT,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT = AUDIO_CMD_TYPE_ELEMENT_ROUTING_CONNECT,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT = AUDIO_CMD_TYPE_ELEMENT_ROUTING_DISCONNECT,
	HRPN_RESP_TYPE_AUDIO_ELEMENT_ROUTING = AUDIO_RESP_TYPE_ELEMENT_ROUTING,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ENABLE = AUDIO_CMD_TYPE_ELEMENT_PLL_ENABLE,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_DISABLE = AUDIO_CMD_TYPE_ELEMENT_PLL_DISABLE,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ID = AUDIO_CMD_TYPE_ELEMENT_PLL_ID,
	HRPN_RESP_TYPE_AUDIO_ELEMENT_PLL = AUDIO_RESP_TYPE_ELEMENT_PLL,

	HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_CONNECT = AUDIO_CMD_TYPE_ELEMENT_AVTP_SOURCE_CONNECT,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_DISCONNECT = AUDIO_CMD_TYPE_ELEMENT_AVTP_SOURCE_DISCONNECT,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SINK_CONNECT = AUDIO_CMD_TYPE_ELEMENT_AVTP_SINK_CONNECT,
	HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SINK_DISCONNECT = AUDIO_CMD_TYPE_ELEMENT_AVTP_SINK_DISCONNECT,
	HRPN_RESP_TYPE_AUDIO_ELEMENT_AVTP = AUDIO_RESP_TYPE_ELEMENT_AVTP,

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
	bool quiet;
};

struct hrpn_cmd_latency_stop {
	uint32_t type;
};

struct hrpn_resp_latency {
	uint32_t type;
	uint32_t status;
};

/* Industrial application commands */
struct hrpn_cmd_industrial_run {
	uint32_t type;
	uint32_t mode;
	uint32_t role;
	uint32_t protocol;
	uint32_t period;
	uint32_t num_io_devices;
	uint32_t control_strategy;
	uint32_t app_mode;
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
		struct audio_cmd_run audio_run;
		struct audio_cmd_stop audio_stop;
		struct audio_cmd_pipeline audio_pipeline;
		struct hrpn_cmd_industrial_run industrial_run;
		struct hrpn_cmd_industrial_stop industrial_stop;
		struct hrpn_cmd_ethernet ethernet;
	} u;
};

struct hrpn_response {
	union {
		struct hrpn_resp resp;
		struct hrpn_resp_latency latency;
		struct audio_resp audio;
		struct hrpn_resp_industrial industrial;
	} u;
};

#endif /* _HRPN_CTRL_H_ */
