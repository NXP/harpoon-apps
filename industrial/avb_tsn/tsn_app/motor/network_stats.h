/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NETWORK_STATS_H_
#define _NETWORK_STATS_H_

#define NUM_MSG_PER_FRAME 4
#define NUM_MOTORS        2

struct net_stat_msg {
    uint16_t id;
    uint32_t seqid;
    uint64_t demo_count;
    float pos_real;
    float pos_target;
    float speed_real;
    float speed_target_ff;
    float speed_out_pos_err;
    float iq_req;
    float iq_real;
    float id_real;
    float uq_applied_fb;
    float dc_bus_fb;
} __attribute__((packed));

struct net_stat_frame {
    struct net_stat_msg stats[NUM_MSG_PER_FRAME];
};

struct network_stats_ctx;

#if ENABLE_LWIP == 1
int network_stats_open(struct network_stats_ctx **ctx);
int network_stats_send(struct network_stats_ctx *ctx, struct net_stat_msg *dg);
#else
static inline int network_stats_open(struct network_stats_ctx **ctx)
{
    return 0;
}

static inline int network_stats_send(struct network_stats_ctx *ctx, struct net_stat_msg *dg)
{
    return 0;
}
#endif

#endif /* _NETWORK_STATS_H_ */
