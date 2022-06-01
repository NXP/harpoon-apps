/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _HRPN_CTRL_AUDIO_PIPELINE_H_
#define _HRPN_CTRL_AUDIO_PIPELINE_H_

#include <stdint.h>

/* Audio pipeline commands */
struct hrpn_cmd_audio_pipeline_id {
	uint32_t id;		/* pipeline id */
};

struct hrpn_cmd_audio_element_id {
	uint32_t type;		/* element type */
	uint32_t id;		/* element id, for the given type */
};

struct hrpn_cmd_audio_element_common {
	uint32_t type;		/* command type */
	struct hrpn_cmd_audio_pipeline_id pipeline;
	struct hrpn_cmd_audio_element_id element;
};

struct hrpn_resp_audio_element_routing {
	uint32_t type;		/* command type */
	uint32_t status;
};

struct hrpn_cmd_audio_element_routing_disconnect {
	uint32_t type;		/* command type */
	struct hrpn_cmd_audio_pipeline_id pipeline;
	struct hrpn_cmd_audio_element_id element;
	uint32_t output;
};

struct hrpn_cmd_audio_element_routing_connect {
	uint32_t type;		/* command type */
	struct hrpn_cmd_audio_pipeline_id pipeline;
	struct hrpn_cmd_audio_element_id element;
	uint32_t output;
	uint32_t input;
};

struct hrpn_cmd_audio_element_routing {
	union {
		struct hrpn_cmd_audio_element_common common;
		struct hrpn_cmd_audio_element_routing_connect connect;
		struct hrpn_cmd_audio_element_routing_disconnect disconnect;
	} u;
};

struct hrpn_cmd_audio_element_pll {
	union {
		struct hrpn_cmd_audio_element_common common;
	} u;
};

struct hrpn_cmd_audio_element_dump {
	uint32_t type;		/* command type */
	struct hrpn_cmd_audio_pipeline_id pipeline;
	struct hrpn_cmd_audio_element_id element;
};

struct hrpn_resp_audio_element {
	uint32_t type;		/* command type */
	uint32_t status;
};

struct hrpn_cmd_audio_element {
	union {
		struct hrpn_cmd_audio_element_common common;
		struct hrpn_cmd_audio_element_routing routing;
		struct hrpn_cmd_audio_element_pll pll;
		struct hrpn_cmd_audio_element_dump dump;
	} u;
};

struct hrpn_cmd_audio_pipeline_common {
	uint32_t type;		/* command type */
	struct hrpn_cmd_audio_pipeline_id pipeline;
};

struct hrpn_cmd_audio_pipeline_dump {
	uint32_t type;		/* command type */
	struct hrpn_cmd_audio_pipeline_id pipeline;
};

struct hrpn_resp_audio_pipeline {
	uint32_t type;		/* command type */
	uint32_t status;
};

struct hrpn_cmd_audio_pipeline {
	union {
		struct hrpn_cmd_audio_pipeline_common common;
		struct hrpn_cmd_audio_pipeline_dump audio_pipeline_dump;
		struct hrpn_cmd_audio_element element;
	} u;
};

#endif /* _HRPN_CTRL_AUDIO_PIPELINE_H_ */
