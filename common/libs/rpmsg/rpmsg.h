/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RPMSG_H_
#define _RPMSG_H_

#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

struct rpmsg_instance {
	struct rpmsg_lite_instance *volatile rl_inst;
	void *mbox_va;
	void *rpmsg_shmem_va;
	void *rpmsg_buf_va;
};

struct rpmsg_ept {
	struct rpmsg_lite_endpoint *rl_ept;
	struct rpmsg_instance *ri;
	rpmsg_queue_handle ept_q;
	uint32_t remote_addr;
	const char *sn;
};

struct rpmsg_instance *rpmsg_init(int link_id, bool is_coherent);
void rpmsg_deinit(struct rpmsg_instance *ri);
struct rpmsg_ept *rpmsg_create_ept(struct rpmsg_instance *ri, int ept_addr, const char *sn);
int rpmsg_destroy_ept(struct rpmsg_ept *ept);
int rpmsg_send(struct rpmsg_ept *ept, void *data, uint32_t len);
int rpmsg_recv(struct rpmsg_ept *ept, void *data, uint32_t *len);
struct rpmsg_ept *rpmsg_transport_init(int link_id, int ept_addr, const char *sn);

#endif /* _RPMSG_H_ */
