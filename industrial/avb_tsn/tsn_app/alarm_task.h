/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ALARM_TASK_H_
#define _ALARM_TASK_H_

#include "FreeRTOS.h"
#include "queue.h"

#include "tsn_task.h"
#include "tsn_tasks_config.h"

struct alarm_task {
    struct tsn_task *task;
    struct tsn_task_params params;
    int type;
    int id;
    int num_peers;
    int stream_id;
    struct {
        QueueHandle_t handle;
        UBaseType_t length;
    } queue;
    void (*net_rx_func)(void *ctx, int msg_id, int src_id, void *buf, int len);
    void *ctx;
};

int alarm_task_monitor_init(struct alarm_task *a_task,
                            void (*net_rx_func)(void *ctx, int msg_id, int src_id, void *buf, int len),
                            void *ctx);
void alarm_task_monitor_exit(struct alarm_task *a_task);
int alarm_task_io_init(struct alarm_task *a_task);
void alarm_task_io_exit(struct alarm_task *a_task);
int alarm_net_transmit(struct alarm_task *a_task, int msg_id, void *buf, int len);

#endif /* _ALARM_TASK_H_ */

