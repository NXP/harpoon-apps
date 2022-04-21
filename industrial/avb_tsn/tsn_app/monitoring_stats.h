/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MONITORING_STATS_H_
#define _MONITORING_STATS_H_

#define MONITOR_MAX_SOCKET 2
#define MONITOR_MAX_MOTOR  2

enum monitoring_msg_type {
    MSG_CTRLER = 0x0,
    MSG_IO_DEVICE = 0x1,
};

struct monitoring_msg_network {
    uint32_t socket_traffic_latency_max[MONITOR_MAX_SOCKET];
    uint32_t socket_traffic_latency_min[MONITOR_MAX_SOCKET];
};

struct monitoring_msg_cyclic_task {
    uint32_t sched_err_max;
    struct monitoring_msg_network net_stats;
} __attribute__((packed));

struct monitoring_msg_motor {
    uint16_t id;
    int32_t pos_error;
} __attribute__((packed));

struct monitoring_msg {
    uint16_t id;
    uint8_t msg_type;
    uint8_t state;
    struct monitoring_msg_cyclic_task cyclic_task_stats;
    struct monitoring_msg_motor motor_stats[MONITOR_MAX_MOTOR];
} __attribute__((packed));

struct monitoring_stats_ctx;

#if ENABLE_LWIP == 1
int monitoring_stats_open(struct monitoring_stats_ctx **ctx);
int monitoring_stats_send(struct monitoring_stats_ctx *ctx, struct monitoring_msg *datagram);
#else
static inline int monitoring_stats_open(struct monitoring_stats_ctx **ctx) {
    return 0;
}

static inline int monitoring_stats_send(struct monitoring_stats_ctx *ctx, struct monitoring_msg *datagram) {
    return 0;
}
#endif

#endif /* _MONITORING_STATS_H_ */

