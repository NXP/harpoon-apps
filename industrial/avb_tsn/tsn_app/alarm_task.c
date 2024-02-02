/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "alarm_task.h"
#include "tsn_tasks_config.h"

#include "avb_tsn/log.h"
#include "avb_tsn/types.h"

static void net_callback(void *data)
{
    struct net_socket *sock = (struct net_socket *)data;
    struct tsn_task *task = container_of(sock, struct tsn_task, sock_rx[sock->id]);
    struct alarm_task *a_task = task->ctx;

    if (rtos_mqueue_send(a_task->queue.handle, &sock, RTOS_NO_WAIT) < 0) {
        ERR("xQueueSendFromISR() failed\n\r");
    }
}

static void main_alarm_monitor(void *data)
{
    struct alarm_task *a_task = data;

    while (true) {
        struct net_socket *sock;

        if (rtos_mqueue_receive(a_task->queue.handle, &sock, RTOS_MS_TO_TICKS(10000)) < 0)
            continue;

        while (tsn_net_receive_sock(sock) == NET_OK) {
            struct tsn_common_hdr *hdr = sock->buf;

            INF("alarm received from device: %u, time: %llu\n",
                hdr->src_id, hdr->sched_time);

            if (a_task->net_rx_func)
                a_task->net_rx_func(a_task->ctx, hdr->msg_id, hdr->src_id, hdr + 1, hdr->len);
        }
        tsn_net_receive_enable_cb(sock);
    }
}

int alarm_net_transmit(struct alarm_task *a_task, int msg_id, void *buf, int len)
{
    struct tsn_task *task = a_task->task;
    struct net_socket *sock = &task->sock_tx[0];
    struct tsn_common_hdr *hdr = sock->buf;
    int total_len = (len + sizeof(*hdr));
    int status;
    uint64_t now = 0;

    if (total_len >= task->params->tx_buf_size)
        goto err;

    genavb_clock_gettime64(task->params->clk_id, &now);

    hdr->msg_id = msg_id;
    hdr->src_id = a_task->id;
    hdr->len = len;
    hdr->sched_time = now;

    if (len)
        memcpy(hdr + 1, buf, len);

    sock->len = total_len;

    status = tsn_net_transmit_sock(sock);
    if (status != NET_OK)
        goto err;

    return 0;

err:
    return -1;
}

int alarm_task_monitor_init(struct alarm_task *a_task,
                            void (*net_rx_func)(void *ctx, int msg_id, int src_id, void *buf, int len),
                            void *ctx)
{
    struct tsn_task_params *params = &a_task->params;
    struct tsn_stream *rx_stream;
    int rc;

    rx_stream = tsn_conf_get_stream(a_task->stream_id);
    if (!rx_stream)
        goto err;

    memcpy(&params->rx_params[0].addr, &rx_stream->address,
           sizeof(struct net_address));
    params->rx_params[0].addr.port = 0;
    params->num_rx_socket = 1;

    a_task->queue.handle = rtos_mqueue_alloc_init(a_task->queue.length,
                                                  sizeof(struct net_socket *));
    if (!a_task->queue.handle) {
        ERR("rtos_mqueue_alloc_init failed\n");
        goto err;
    }

    rc = tsn_task_register(&a_task->task, params, a_task->id, main_alarm_monitor, a_task, NULL);
    if (rc < 0) {
        ERR("tsn_task_register rc = %d\n", __func__, rc);
        goto err;
    }

    rc = tsn_net_receive_set_cb(&a_task->task->sock_rx[0], net_callback);
    if (rc < 0) {
        ERR("tsn_net_receive_set_cb rc = %d\n", __func__, rc);
        goto err;
    }

    INF("success\n");

    return 0;

err:
    if (a_task->queue.handle)
        rtos_mqueue_destroy(a_task->queue.handle);

    return -1;
}

void alarm_task_monitor_exit(struct alarm_task *a_task)
{
    tsn_task_unregister(&a_task->task);

    if (a_task->queue.handle)
        rtos_mqueue_destroy(a_task->queue.handle);
}

static void main_alarm_io(void *data)
{
    struct alarm_task *a_task = data;

    while (true) {
        rtos_sleep(RTOS_MS_TO_TICKS(10000));
        alarm_net_transmit(a_task, 0, NULL, 0);
    }
}

int alarm_task_io_init(struct alarm_task *a_task)
{
    struct tsn_task_params *params = &a_task->params;
    struct tsn_stream *tx_stream;
    int rc;

    tx_stream = tsn_conf_get_stream(a_task->stream_id);
    if (!tx_stream)
        goto err;

    memcpy(&params->tx_params[0].addr, &tx_stream->address,
           sizeof(struct net_address));
    params->tx_params[0].addr.port = 0;
    params->num_tx_socket = 1;

    rc = tsn_task_register(&a_task->task, params, a_task->id, main_alarm_io, a_task, NULL);
    if (rc < 0) {
        ERR("tsn_task_register rc = %d\n", __func__, rc);
        goto err;
    }

    return 0;

err:
    return -1;
}

void alarm_task_io_exit(struct alarm_task *a_task)
{
    tsn_task_unregister(&a_task->task);
}
