/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/mmu.h"
#include "os/stdlib.h"
#include "os/stdio.h"
#include "os/irq.h"

#include "rpmsg.h"

#define RPMSG_LITE_SHMEM_BASE	(VDEV0_VRING_BASE)

int rpmsg_send(struct rpmsg_ept *ept, void *data, uint32_t len)
{
	int32_t ret;

	ret = rpmsg_lite_send(ept->ri->rl_inst, ept->rl_ept, ept->remote_addr, data, len, RL_BLOCK);

	return ret;
}

int rpmsg_recv(struct rpmsg_ept *ept, void *data, uint32_t len)
{
	uint32_t msg_src_addr;
	uint32_t size;
	int32_t ret;

	ret = rpmsg_queue_recv(ept->ri->rl_inst, ept->ept_q, (uint32_t *)&msg_src_addr, (char *)data, len, &size, RL_DONT_BLOCK);
	if (ret != RL_SUCCESS) {
		if (ret != RL_ERR_NO_BUFF)
			os_printf("rpmsg_queue_recv() failed\n");

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

	ept = os_malloc(sizeof(struct rpmsg_ept));
	if (!ept)
		return ept;

	ept->ri = ri;
	ept->sn = sn;
	ept->remote_addr =  RL_ADDR_ANY;
	ept->ept_q = rpmsg_queue_create(ri->rl_inst);
	if (!ept->ept_q)
	{
		os_printf("rpmsg failed to create ept queue\r\n");
		goto err_create_q;
	}

	ept->rl_ept = rpmsg_lite_create_ept(ri->rl_inst, ept_addr, rpmsg_queue_rx_cb, ept->ept_q);

	if (!ept->rl_ept) {
		os_printf("rpmsg failed to create ept\r\n");
		goto err_create;
	}

	ret = rpmsg_ns_announce(ri->rl_inst, ept->rl_ept, sn, RL_NS_CREATE);
	if (ret != RL_SUCCESS) {
		os_printf("\r\nNameservice: Create channel failed\r\n");
		goto err_announce;
	}

	return ept;

err_announce:
	rpmsg_lite_destroy_ept(ri->rl_inst, ept->rl_ept);
	ept->rl_ept = NULL;

err_create:
	rpmsg_queue_destroy(ri->rl_inst, ept->ept_q);
err_create_q:
	os_free(ept);

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
		os_free(ept);
	}

	return ret;
}

static void rpmsg_mailbox_init(void)
{
	os_irq_register(RL_GEN_SW_MBOX_IRQ, gen_sw_mbox_handler,
			(void *)RL_GEN_SW_MBOX_BASE, OS_IRQ_PRIO_DEFAULT);
	os_irq_enable(RL_GEN_SW_MBOX_IRQ);
}

struct rpmsg_instance *rpmsg_init(int link_id)
{
	struct rpmsg_instance *ri;
	int ret;

	ri = os_malloc(sizeof(struct rpmsg_instance));
	if (!ri)
		return ri;

	ret = os_mmu_map("MBOX", (uint8_t **)&ri->mbox_va,
			(uintptr_t)RL_GEN_SW_MBOX_BASE, KB(4),
			OS_MEM_DEVICE_nGnRE | OS_MEM_PERM_RW);
	os_assert(ret == 0, "os_mmu_map() failed\n");
	ret = os_mmu_map("RPMSG", (uint8_t **)&ri->rpmsg_shmem_va,
			(uintptr_t)RPMSG_LITE_SHMEM_BASE, KB(64),
			OS_MEM_DEVICE_nGnRE | OS_MEM_PERM_RW);
	os_assert(ret == 0, "os_mmu_map() failed\n");
	ret = os_mmu_map("VRINGBUF", (uint8_t **)&ri->rpmsg_buf_va,
			(uintptr_t)RPMSG_BUF_BASE, MB(1),
			OS_MEM_CACHE_NONE | OS_MEM_PERM_RW);
	os_assert(ret == 0, "os_mmu_map() failed\n");

	rpmsg_mailbox_init();

	os_printf("\r\nRPMSG init ...\r\n");
	ri->rl_inst = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, link_id, RL_NO_FLAGS);
	if (!ri->rl_inst) {
		os_free(ri);
		return NULL;
	}

	rpmsg_lite_wait_for_link_up(ri->rl_inst);

	os_printf("\r\nRPMSG link up\r\n");

	return ri;
}

void rpmsg_deinit(struct rpmsg_instance *ri)
{
	rpmsg_lite_deinit(ri->rl_inst);
	os_mmu_unmap((uintptr_t)ri->rpmsg_buf_va, MB(1));
	os_mmu_unmap((uintptr_t)ri->rpmsg_shmem_va, KB(64));
	os_mmu_unmap((uintptr_t)ri->mbox_va, KB(4));
	os_free(ri);
}

int rpmsg_transport_init(int link_id, int ept_addr, const char *sn,
				void **tp, void **cmd, void **resp)
{
	struct rpmsg_instance *ri;
	struct rpmsg_ept *ept;

	ri = rpmsg_init(link_id);
	os_assert(ri, "rpmsg initialization failed, cannot proceed\n");
	ept = rpmsg_create_ept(ri, ept_addr, sn);
	os_assert(ept, "rpmsg ept creation failed, cannot proceed\n");
	*tp = ept;
	*cmd = os_malloc(1024);
	os_assert(*cmd, "malloc mailbox memory failded, cannot proceed\n");
	*resp = *cmd + 512;
	memset(*cmd, 0, 1024);

	return 0;
}