/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_ENTRY_H_
#define _INDUSTRIAL_ENTRY_H_

#include "ivshmem.h"
#include "mailbox.h"

#include "industrial_os.h"

#include "os/mqueue.h"
#include "os/semaphore.h"

enum industrial_use_case_id {
	INDUSTRIAL_USE_CASE_CAN = 0,
	INDUSTRIAL_USE_CASE_ETHERNET,

	INDUSTRIAL_USE_CASE_MAX,
};

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

	os_sem_t semaphore;
	os_mqd_t mqueue;

	const struct mode_operations *ops;

	void (*process_data)(void *priv);
	void *priv;
};

struct ctrl_ctx {
	struct ivshmem mem;
	struct mailbox mb;
};

struct industrial_ctx {
	struct ctrl_ctx ctrl;

	struct data_ctx data[INDUSTRIAL_USE_CASE_MAX];
};

struct industrial_use_case {
	struct thread_cfg thread;

	/*
	 * Only one mode of operation for now (MCU SDK API, or full stack, ...)
	 * Possibility to add more in the future if we decide to have more than
	 * one software path for a given hardware resource.
	 */
	struct mode_operations ops[3];
};

void *can_init_loopback(void *parameters);
void *can_init_interrupt(void *parameters);
void *can_init_pingpong(void *parameters);
int can_run(void *priv, struct event *e);
void can_stats(void *priv);
void can_exit(void *priv);

void *ethernet_avb_tsn_init(void *parameters);
int ethernet_avb_tsn_run(void *priv, struct event *e);
void ethernet_avb_tsn_stats(void *priv);
void ethernet_avb_tsn_exit(void *priv);

void *ethernet_sdk_enet_init(void *parameters);
int ethernet_sdk_enet_run(void *priv, struct event *e);
void ethernet_sdk_enet_stats(void *priv);
void ethernet_sdk_enet_exit(void *priv);

void *industrial_control_init(int nb_use_cases);
void *industrial_get_data_ctx(struct industrial_ctx *ctx, int use_case_id);
void industrial_control_loop(void *context);

#endif /* _INDUSTRIAL_ENTRY_H_ */
