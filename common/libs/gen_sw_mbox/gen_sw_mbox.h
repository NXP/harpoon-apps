/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GEN_SW_MBOX_H_
#define GEN_SW_MBOX_H_

void gen_sw_mbox_init(void);
void gen_sw_mbox_deinit(void);
int gen_sw_mbox_register(void *base, int irq, int remote_irq, uint32_t irq_prio);
int gen_sw_mbox_unregister(void *base);
int gen_sw_mbox_register_chan_callback(void *base, uint32_t ch, void (*recv_cb)(void *data, uint32_t msg), void *data);
int gen_sw_mbox_unregister_chan_callback(void *base, uint32_t ch);
int gen_sw_mbox_sendmsg(void *base, uint32_t ch, uint32_t msg, bool block);

#endif /* GEN_SW_MBOX_H_ */
