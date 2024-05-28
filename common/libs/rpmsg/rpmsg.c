/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "hlog.h"
#include "os/mmu.h"
#include "os/stdio.h"
#include "os/irq.h"

#include "rpmsg.h"
#include "gen_sw_mbox.h"
#include "gen_sw_mbox_config.h"
#include "rtos_abstraction_layer.h"

#define RPMSG_LITE_SHMEM_BASE	(VDEV0_VRING_BASE)

int rpmsg_send(struct rpmsg_ept *ept, void *data, uint32_t len)
{
	int32_t ret;

	ret = rpmsg_lite_send(ept->ri->rl_inst, ept->rl_ept, ept->remote_addr, data, len, RL_BLOCK);

	return ret;
}

int rpmsg_recv(struct rpmsg_ept *ept, void *data, uint32_t *len)
{
	uint32_t msg_src_addr;
	int32_t ret;

	ret = rpmsg_queue_recv(ept->ri->rl_inst, ept->ept_q, (uint32_t *)&msg_src_addr, (char *)data, *len, len, RL_DONT_BLOCK);
	if (ret != RL_SUCCESS) {
		if (ret != RL_ERR_NO_BUFF)
			log_err("rpmsg_queue_recv() failed\n");

		return ret;
	}

	if (ept->remote_addr ==  RL_ADDR_ANY)
		ept->remote_addr = msg_src_addr;
	else if (ept->remote_addr != msg_src_addr)
		return -1;

	return ret;
}

struct rpmsg_ept *rpmsg_create_ept(struct rpmsg_instance *ri, int ept_addr, const char *sn)
{
	struct rpmsg_ept *ept;
	int ret;

	ept = rtos_malloc(sizeof(struct rpmsg_ept));
	if (!ept)
		return ept;

	ept->ri = ri;
	ept->sn = sn;
	ept->remote_addr =  RL_ADDR_ANY;
	ept->ept_q = rpmsg_queue_create(ri->rl_inst);
	if (!ept->ept_q)
	{
		log_err("rpmsg failed to create ept queue\n");
		goto err_create_q;
	}

	ept->rl_ept = rpmsg_lite_create_ept(ri->rl_inst, ept_addr, rpmsg_queue_rx_cb, ept->ept_q);

	if (!ept->rl_ept) {
		log_err("rpmsg failed to create ept\n");
		goto err_create;
	}

	ret = rpmsg_ns_announce(ri->rl_inst, ept->rl_ept, sn, RL_NS_CREATE);
	if (ret != RL_SUCCESS) {
		log_err("Nameservice: Create channel failed\n");
		goto err_announce;
	}

	return ept;

err_announce:
	rpmsg_lite_destroy_ept(ri->rl_inst, ept->rl_ept);
	ept->rl_ept = NULL;

err_create:
	rpmsg_queue_destroy(ri->rl_inst, ept->ept_q);
err_create_q:
	rtos_free(ept);

	return NULL;
}

int rpmsg_destroy_ept(struct rpmsg_ept *ept)
{
	int ret;

	ret = rpmsg_ns_announce(ept->ri->rl_inst, ept->rl_ept, ept->sn, RL_NS_DESTROY);

	if (ret != RL_SUCCESS)
		return ret;

	ret = rpmsg_lite_destroy_ept(ept->ri->rl_inst, ept->rl_ept);
	if (ret == RL_SUCCESS)
	{
		rpmsg_queue_destroy(ept->ri->rl_inst, ept->ept_q);
		rtos_free(ept);
	}

	return ret;
}

static void rpmsg_mailbox_init(void)
{
	gen_sw_mbox_init();
	gen_sw_mbox_register((void *)GEN_SW_MBOX_BASE, GEN_SW_MBOX_IRQ,
			     GEN_SW_MBOX_REMOTE_IRQ, GEN_SW_MBOX_IRQ_PRIO);
}

struct rpmsg_instance *rpmsg_init(int link_id, bool is_coherent)
{
	struct rpmsg_instance *ri;

	ri = rtos_malloc(sizeof(struct rpmsg_instance));
	if (!ri)
		return ri;

	if (os_mmu_map("RPMSG", (uint8_t **)&ri->rpmsg_shmem_va,
			(uintptr_t)RPMSG_LITE_SHMEM_BASE, KB(64),
			(is_coherent ? OS_MEM_CACHE_WB : OS_MEM_DEVICE_nGnRE) |
			OS_MEM_PERM_RW)) {
		log_err("RPMSG os_mmu_map() failed\n");

		goto err_map_rpmsg;
	}

	if (os_mmu_map("VRINGBUF", (uint8_t **)&ri->rpmsg_buf_va,
			(uintptr_t)RPMSG_BUF_BASE, MB(1),
			(is_coherent ? OS_MEM_CACHE_WB : OS_MEM_CACHE_NONE) |
			OS_MEM_PERM_RW | OS_MEM_DIRECT_MAP)) {
		log_err("VRINGBUF os_mmu_map() failed\n");

		goto err_map_vringbuf;
	}

	rpmsg_mailbox_init();

	log_info("RPMSG init ...\n");
	ri->rl_inst = rpmsg_lite_remote_init(ri->rpmsg_shmem_va, link_id, RL_NO_FLAGS);
	if (!ri->rl_inst) {
		log_err("rpmsg_lite_remote_init() failed\n");
		goto err_rpmsg_lite_init;
	}

	rpmsg_lite_wait_for_link_up(ri->rl_inst, RL_BLOCK);

	log_info("RPMSG link up\n");

	return ri;

err_rpmsg_lite_init:
	os_mmu_unmap((uintptr_t)ri->rpmsg_buf_va, MB(1));
err_map_vringbuf:
	os_mmu_unmap((uintptr_t)ri->rpmsg_shmem_va, KB(64));
err_map_rpmsg:
	rtos_free(ri);

	return NULL;
}

void rpmsg_deinit(struct rpmsg_instance *ri)
{
	rpmsg_lite_deinit(ri->rl_inst);
	os_mmu_unmap((uintptr_t)ri->rpmsg_buf_va, MB(1));
	os_mmu_unmap((uintptr_t)ri->rpmsg_shmem_va, KB(64));
	rtos_free(ri);
}

struct rpmsg_ept *rpmsg_transport_init(int link_id, int ept_addr, const char *sn)
{
	struct rpmsg_instance *ri;
	struct rpmsg_ept *ept;

	ri = rpmsg_init(link_id, true);
	if (!ri) {
		log_err("rpmsg_init() failed\n");
		goto err_rpmsg_init;
	}

	ept = rpmsg_create_ept(ri, ept_addr, sn);
	if (!ept) {
		log_err("rpmsg_create_ept() failed\n");
		goto err_rpmsg_create_ept;
	}

	return ept;

err_rpmsg_create_ept:
	rpmsg_deinit(ri);
err_rpmsg_init:
	return NULL;
}
