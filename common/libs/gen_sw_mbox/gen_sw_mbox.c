/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "os/assert.h"
#include "os/irq.h"
#include "os/mmu.h"
#include "os/semaphore.h"
#include "os/stdlib.h"
#include "fsl_device_registers.h"

#define MBOX_MAX_INST	(4)
#define MAX_CH		(4)

/*
 * Generic software mailbox Registers:
 *
 * RX_STATUS[n]: RX channel n status
 * TX_STATUS[n]: TX channel n status
 * 	0: indicates message in T/RX_CH[n] is invalid and channel ready.
 * 	1: indicates message in T/RX_CH[n] is valid and channel busy.
 * 	2: indicates message in T/RX_CH[n] has been received by the peer.
 * RX_CH[n]: Receive data register for channel n
 * TX_CH[n]: Transmit data register for channel n
 *
 * To send a message:
 * Update the data register TX_CH[n] with the message, then set the
 * TX_STATUS[n] to 1, inject a interrupt to remote side; after the
 * transmission done set the TX_STATUS[n] back to 0.
 *
 * When received a message:
 * Get the received data from RX_CH[n] and then set the RX_STATUS[n] to
 * 2 and inject a interrupt to notify the remote side transmission done.
 */

#ifdef GEN_SW_MBOX_MASTER
struct gen_sw_mbox_mmio {
	volatile uint32_t tx_status[MAX_CH];
	volatile uint32_t rx_status[MAX_CH];
	uint32_t reserved_1[MAX_CH];
	volatile uint32_t tx_ch[MAX_CH];
	volatile uint32_t rx_ch[MAX_CH];
	uint32_t reserved_2[MAX_CH];
	volatile uint32_t ch_ack_flags; /* from bit0, each bit for one channel: rx_ch_0..rx_ch_MAX_CH, tx_ch_0..tx_ch_MX_CH */
};
#else
struct gen_sw_mbox_mmio {
	volatile uint32_t rx_status[MAX_CH];
	volatile uint32_t tx_status[MAX_CH];
	uint32_t reserved_1[MAX_CH];
	volatile uint32_t rx_ch[MAX_CH];
	volatile uint32_t tx_ch[MAX_CH];
	uint32_t reserved_2[MAX_CH];
	volatile uint32_t ch_ack_flags; /* from bit0, each bit for one channel: rx_ch_0..rx_ch_MAX_CH, tx_ch_0..tx_ch_MX_CH */
};
#endif

enum gen_sw_mbox_chan_status {
	S_READY,
	S_BUSY,
	S_DONE,
};

struct gen_sw_mbox_chan {
	uint32_t id;
	void (*recv_cb)(void *data, uint32_t msg);
	void *data;
};

struct gen_sw_mbox {
	void *mmio_pa;
	struct gen_sw_mbox_mmio *mmio;
	int irq, remote_irq;
	struct gen_sw_mbox_chan chan[MAX_CH];
	int ref_cnt;
	os_sem_t lock;
};

static struct gen_sw_mbox *mbox_inst[MBOX_MAX_INST];

static os_sem_t gen_sw_mbox_semaphore;

static bool is_inited;

static struct gen_sw_mbox *gen_sw_mbox_get_instance(void *base)
{
	int i;

	for (i = 0; i < MBOX_MAX_INST; i++) {
		if (mbox_inst[i] && mbox_inst[i]->mmio_pa == base) {
			return mbox_inst[i];
		}
	}

	return NULL;
}

static void gen_sw_mbox_get(struct gen_sw_mbox *mbox)
{
	os_assert(mbox, "gen_sw_mbox instance is NULL!");

	os_sem_take(&mbox->lock, 0, OS_SEM_TIMEOUT_MAX);
	mbox->ref_cnt++;
	os_sem_give(&mbox->lock, 0);
}

static void gen_sw_mbox_put(struct gen_sw_mbox *mbox)
{
	os_assert(mbox, "gen_sw_mbox instance is NULL!");

	os_sem_take(&mbox->lock, 0, OS_SEM_TIMEOUT_MAX);
	mbox->ref_cnt--;
	os_sem_give(&mbox->lock, 0);
}

static void gen_sw_mbox_handler(void *data)
{
	struct gen_sw_mbox *mbox = data;
	struct gen_sw_mbox_mmio *mmio = mbox->mmio;
	uint32_t msg;
	int i;

	for (i = 0; i < MAX_CH; i++) {
		/* Handle TX done ack */
		if (mmio->tx_status[i] == S_DONE)
			mmio->tx_status[i] = S_READY;

		/* Skip idle channels */
		if (mmio->rx_status[i] != S_BUSY)
			continue;

		msg = mmio->rx_ch[i];
		__DSB();
		mmio->rx_status[i] = S_DONE;

		GIC_SetPendingIRQ(mbox->remote_irq);

		if (mbox->chan[i].recv_cb)
			mbox->chan[i].recv_cb(mbox->chan[i].data, msg);

		if (mmio->ch_ack_flags & (1 << i)) {
			/* Need ACK */
			mmio->rx_status[i] = S_DONE;
			GIC_SetPendingIRQ(mbox->remote_irq);
		} else
			/* No ACK */
			mmio->rx_status[i] = S_READY;
	}
}

int gen_sw_mbox_sendmsg(void *base, uint32_t ch, uint32_t msg, bool block)
{
	struct gen_sw_mbox *mbox;
	struct gen_sw_mbox_mmio *mmio;

	os_assert(ch < MAX_CH, "gen_sw_mbox channel is invalid!");

	os_sem_take(&gen_sw_mbox_semaphore, 0, OS_SEM_TIMEOUT_MAX);
	mbox = gen_sw_mbox_get_instance(base);
	if (!mbox) {
		os_sem_give(&gen_sw_mbox_semaphore, 0);
		return -1;
	}
	gen_sw_mbox_get(mbox);
	os_sem_give(&gen_sw_mbox_semaphore, 0);

	mmio = mbox->mmio;

	while (mmio->tx_status[ch] != S_READY) {
		if (!block)
			return -2;
	}

	mmio->tx_ch[ch] = msg;
	__DSB();
	mmio->tx_status[ch] = S_BUSY;

	GIC_SetPendingIRQ(mbox->remote_irq);

	gen_sw_mbox_put(mbox);

	return 0;
}

int gen_sw_mbox_register_chan_callback(void *base, uint32_t ch,
				       void (*recv_cb)(void *data, uint32_t msg),
				       void *data)
{
	struct gen_sw_mbox *mbox;

	os_assert(ch < MAX_CH, "gen_sw_mbox channel is invalid!");
	os_assert(recv_cb, "gen_sw_mbox channel recv_cb() is NULL!");

	os_sem_take(&gen_sw_mbox_semaphore, 0, OS_SEM_TIMEOUT_MAX);
	mbox = gen_sw_mbox_get_instance(base);
	if (!mbox) {
		os_sem_give(&gen_sw_mbox_semaphore, 0);
		return -1;
	}

	gen_sw_mbox_get(mbox);
	os_sem_give(&gen_sw_mbox_semaphore, 0);

	mbox->chan[ch].recv_cb = recv_cb;
	mbox->chan[ch].data = data;

	return 0;
}

int gen_sw_mbox_unregister_chan_callback(void *base, uint32_t ch)
{
	struct gen_sw_mbox *mbox;

	os_assert(ch < MAX_CH, "gen_sw_mbox channel is invalid!");

	os_sem_take(&gen_sw_mbox_semaphore, 0, OS_SEM_TIMEOUT_MAX);
	mbox = gen_sw_mbox_get_instance(base);
	if (!mbox) {
		os_sem_give(&gen_sw_mbox_semaphore, 0);
		return -1;
	}

	mbox->chan[ch].recv_cb = NULL;
	mbox->chan[ch].data = NULL;

	gen_sw_mbox_put(mbox);
	os_sem_give(&gen_sw_mbox_semaphore, 0);

	return 0;
}

static int gen_sw_mbox_del_mbox(struct gen_sw_mbox *mbox)
{
	int i;

	for (i = 0; i < MBOX_MAX_INST; i++) {
		if (mbox_inst[i] == mbox) {
			mbox_inst[i] = NULL;
			return 0;
		}
	}

	return -1;
}

static int gen_sw_mbox_add_mbox(struct gen_sw_mbox *mbox)
{
	int i;

	for (i = 0; i < MBOX_MAX_INST; i++) {
		if (!mbox_inst[i]) {
			mbox_inst[i] = mbox;
			return 0;
		}
	}

	return -1;
}

void gen_sw_mbox_init(void)
{
	int err;

	if (is_inited == true)
		return;

	err = os_sem_init(&gen_sw_mbox_semaphore, 1);
	os_assert(!err, "gen_sw_mbox_semaphore initialization failed!");
	is_inited = true;
}

void gen_sw_mbox_deinit(void)
{
	if (is_inited != true)
		return;

	os_sem_destroy(&gen_sw_mbox_semaphore);
	is_inited = false;
}

int gen_sw_mbox_register(void *base, int irq, int remote_irq, uint32_t irq_prio)
{
	struct gen_sw_mbox *mbox;
	int ret;
	int i;

	os_assert(base, "gen_sw_mbox MMIO base is NULL!");

	os_sem_take(&gen_sw_mbox_semaphore, 0, OS_SEM_TIMEOUT_MAX);

	mbox = gen_sw_mbox_get_instance(base);
	if (mbox) {
		ret = 0;
		goto exit;
	}

	mbox = os_malloc(sizeof(struct gen_sw_mbox));
	if (!mbox) {
		ret = -1;
		goto exit;
	}

	mbox->irq = irq;
	mbox->remote_irq = remote_irq;

	mbox->mmio_pa = base;
	if (os_mmu_map("MBOX", (uint8_t **)&mbox->mmio,
				(uintptr_t)base, KB(4),
				OS_MEM_DEVICE_nGnRE | OS_MEM_PERM_RW)) {
		ret = -2;
		goto err_map;
	}

	ret = os_sem_init(&mbox->lock, 1);
	if (ret)
		goto err_semaphore;

	for (i = 0; i < MAX_CH; i++) {
		mbox->mmio->rx_status[i] = 0;
		mbox->mmio->tx_status[i] = 0;

		mbox->chan[i].id = i;
		mbox->chan[i].recv_cb = NULL;
	}

	os_irq_register(irq, gen_sw_mbox_handler, mbox, irq_prio);
	os_irq_enable(irq);

	mbox->ref_cnt = 0;

	gen_sw_mbox_add_mbox(mbox);

	os_sem_give(&gen_sw_mbox_semaphore, 0);

	return 0;

err_semaphore:
	os_mmu_unmap((uintptr_t)mbox->mmio, KB(4));
err_map:
	os_free(mbox);
exit:
	os_sem_give(&gen_sw_mbox_semaphore, 0);

	return ret;
}

int gen_sw_mbox_unregister(void *base)
{
	struct gen_sw_mbox *mbox;

	os_assert(base, "gen_sw_mbox MMIO base is NULL!");

	os_sem_take(&gen_sw_mbox_semaphore, 0, OS_SEM_TIMEOUT_MAX);
	mbox = gen_sw_mbox_get_instance(base);
	if (!mbox) {
		os_sem_give(&gen_sw_mbox_semaphore, 0);
		return 0;
	}

	os_sem_take(&mbox->lock, 0, OS_SEM_TIMEOUT_MAX);
	if (mbox->ref_cnt > 0) {
		os_sem_give(&mbox->lock, 0);
		return -1;
	}

	gen_sw_mbox_del_mbox(mbox);

	os_sem_give(&mbox->lock, 0);
	os_sem_give(&gen_sw_mbox_semaphore, 0);

	os_irq_disable(mbox->irq);
	os_irq_unregister(mbox->irq);
	os_mmu_unmap((uintptr_t)mbox->mmio, KB(4));

	os_sem_destroy(&mbox->lock);
	os_free(mbox);

	return 0;
}
