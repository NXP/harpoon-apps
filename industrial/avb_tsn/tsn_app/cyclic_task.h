/*
 * Copyright 2019, 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CYCLIC_TASK_H_
#define _CYCLIC_TASK_H_

#include "tsn_task.h"
#include "tsn_tasks_config.h"
#include "monitoring_stats.h"

#define CYCLIC_STAT_PERIOD_SEC 5
#define CYCLIC_EVENT_QUEUE_LENGTH 1

struct cyclic_event {
    unsigned int type;
    void *data;
};

enum cyclic_event_type {
    CYCLIC_EVENT_TYPE_TIMER = 0,
};

struct socket_stats {
    bool pending;
    unsigned int valid_frames;
    unsigned int err_id;
    unsigned int err_ts;
    unsigned int err_underflow;
    int link_status;
    unsigned int traffic_latency_max;
    unsigned int traffic_latency_min;
    struct rtos_apps_stats traffic_latency;
    struct rtos_apps_hist traffic_latency_hist;
};

struct socket {
    int peer_id;
    int stream_id;
    struct socket_stats stats;
    struct socket_stats stats_snap;
    struct net_socket *net_sock;
};

struct cyclic_task {
    struct tsn_task *task;
    struct tsn_task_params params;
    int type;
    int id;
    int num_peers;
    struct socket rx_socket[MAX_PEERS];
    struct socket tx_socket;
    void (*net_rx_func)(void *ctx, int msg_id, int src_id, void *buf, int len);
    void (*loop_func)(void *ctx, int timer_status);
    void *ctx;
    rtos_mqueue_t *queue_h;
};

int cyclic_task_init(struct cyclic_task *c_task,
                     void (*net_rx_func)(void *ctx, int msg_id, int src_id, void *buf, int len),
                     void (*loop_func)(void *ctx, int timer_status), void *ctx);
void cyclic_task_exit(struct cyclic_task *c_task);
int cyclic_task_start(struct cyclic_task *);
void cyclic_task_stop(struct cyclic_task *);
int cyclic_net_transmit(struct cyclic_task *c_task, int msg_id, void *buf, int len);
void cyclic_task_get_monitoring(struct cyclic_task *task, struct monitoring_msg_cyclic_task *mon_cyclic_task,
                                uint32_t num_socket_monitored);
void cyclic_task_set_period(struct cyclic_task *c_task, unsigned int period_ns);

#endif /* _CYCLIC_TASK_H_ */
