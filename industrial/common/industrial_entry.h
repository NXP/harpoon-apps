/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_ENTRY_H_
#define _INDUSTRIAL_ENTRY_H_

#include "rpmsg.h"
#include "industrial_os.h"

#include "rtos_abstraction_layer.h"

enum industrial_use_case_id {
	INDUSTRIAL_USE_CASE_CAN = 0,
	INDUSTRIAL_USE_CASE_ETHERNET,

	INDUSTRIAL_USE_CASE_MAX,
};

#define INDUSTRIAL_CAN_USE_CASES_NUM 1
#define INDUSTRIAL_ETHERNET_USE_CASES_NUM 1
#define INDUSTRIAL_USE_CASES_MAX 3

struct event {
	unsigned int type;
	uintptr_t data;
};

struct mode_operations {
	void *(*init)(void *);
	void (*exit)(void *);
	void (*stats)(void *);
	int (*run)(void *, struct event *e);
};

struct data_ctx {

	unsigned int id;

	rtos_mutex_t mutex;
	rtos_mqueue_t *mqueue_h;

	const struct mode_operations *ops;

	void (*process_data)(void *priv);
	void *priv;
};

struct ctrl_ctx {
	struct rpmsg_ept *ept;
};

struct industrial_ctx {
	struct ctrl_ctx ctrl;

	struct data_ctx data[INDUSTRIAL_USE_CASE_MAX];
};

struct industrial_use_case {
	struct thread_cfg thread;

	struct mode_operations ops[INDUSTRIAL_USE_CASES_MAX];
	uint32_t ops_num;
};

void *can_init(void *parameters);
int can_run(void *priv, struct event *e);
void can_exit(void *priv);
void can_stats(void *priv);

void *ethernet_avb_tsn_init(void *parameters);
int ethernet_avb_tsn_run(void *priv, struct event *e);
void ethernet_avb_tsn_stats(void *priv);
void ethernet_avb_tsn_exit(void *priv);

void *industrial_control_init(int nb_use_cases);
void *industrial_get_data_ctx(struct industrial_ctx *ctx, int use_case_id);
void industrial_control_loop(void *context);

#endif /* _INDUSTRIAL_ENTRY_H_ */
