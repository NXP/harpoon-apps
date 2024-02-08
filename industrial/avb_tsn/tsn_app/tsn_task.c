/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tsn_task.h"

#include "avb_tsn/stats_task.h"
#include "avb_tsn/log.h"
#include "avb_tsn/types.h"

#include "genavb.h"
#include "genavb/srp.h"
#include "genavb/qos.h"
#include "genavb/ether.h"

//#define SRP_RESERVATION

void tsn_task_stats_init(struct tsn_task *task)
{
    stats_init(&task->stats.sched_err, 31, "sched err", NULL);
    hist_init(&task->stats.sched_err_hist, 100, 100);

    stats_init(&task->stats.proc_time, 31, "processing time", NULL);
    hist_init(&task->stats.proc_time_hist, 100, 1000);

    stats_init(&task->stats.total_time, 31, "total time", NULL);
    hist_init(&task->stats.total_time_hist, 100, 1000);

    task->stats.sched_err_max = 0;
}

void tsn_task_stats_start(struct tsn_task *task)
{
    uint64_t now = 0;
    int32_t sched_err;

    task->stats.sched++;

    genavb_clock_gettime64(task->params->clk_id, &now);

    sched_err = now - task->sched_time;

    if (sched_err > (2 * task->params->task_period_ns))
        task->stats.sched_missed++;

    if (sched_err < 0) {
        task->stats.sched_early++;
        sched_err = -sched_err;
    }

    stats_update(&task->stats.sched_err, sched_err);
    hist_update(&task->stats.sched_err_hist, sched_err);

    if (sched_err > task->stats.sched_err_max)
        task->stats.sched_err_max = sched_err;

    task->sched_now = now;
}

void tsn_task_stats_end(struct tsn_task *task)
{
    uint64_t now = 0;
    int32_t proc_time;
    int32_t total_time;

    genavb_clock_gettime64(task->params->clk_id, &now);

    proc_time = now - task->sched_now;
    total_time = now - task->sched_time;

    stats_update(&task->stats.proc_time, proc_time);
    hist_update(&task->stats.proc_time_hist, proc_time);

    stats_update(&task->stats.total_time, total_time);
    hist_update(&task->stats.total_time_hist, total_time);

    task->sched_time += task->params->task_period_ns;
}

static void tsn_net_st_oper_config_print(struct tsn_task *task);
static void tsn_net_fp_print(struct tsn_task *task);

static void tsn_task_stats_print(void *data)
{
    struct tsn_task *task = data;

    stats_compute(&task->stats_snap.sched_err);
    stats_compute(&task->stats_snap.proc_time);
    stats_compute(&task->stats_snap.total_time);

    INF("tsn task(%p)\n", task);
    INF("sched           : %u\n", task->stats_snap.sched);
    INF("sched early     : %u\n", task->stats_snap.sched_early);
    INF("sched missed    : %u\n", task->stats_snap.sched_missed);
    INF("sched timeout   : %u\n", task->stats_snap.sched_timeout);
    INF("clock discont   : %u\n", task->stats_snap.clock_discont);

    stats_print(&task->stats_snap.sched_err);
    hist_print(&task->stats_snap.sched_err_hist);

    stats_print(&task->stats_snap.proc_time);
    hist_print(&task->stats_snap.proc_time_hist);

    stats_print(&task->stats_snap.total_time);
    hist_print(&task->stats_snap.total_time_hist);

    if (task->params->use_st)
        tsn_net_st_oper_config_print(task);

    if (task->params->use_fp)
        tsn_net_fp_print(task);

    task->stats_snap.pending = false;
}

static void tsn_task_stats_dump(struct tsn_task *task)
{
    if (task->stats_snap.pending)
        return;

    memcpy(&task->stats_snap, &task->stats, sizeof(struct tsn_task_stats));
    stats_reset(&task->stats.sched_err);
    stats_reset(&task->stats.proc_time);
    stats_reset(&task->stats.total_time);
    task->stats_snap.pending = true;

    if (STATS_Async(tsn_task_stats_print, task) < 0)
        task->stats_snap.pending = false;
}

static void net_socket_stats_print(void *data)
{
    struct net_socket *sock = data;

    INF("net %s socket(%p) %d\n", sock->dir ? "tx" : "rx", sock, sock->id);
    INF("frames     : %u\n", sock->stats_snap.frames);
    INF("err        : %u\n", sock->stats_snap.err);

    sock->stats_snap.pending = false;
}

void net_socket_stats_dump(struct net_socket *sock)
{
    if (sock->stats_snap.pending)
        return;

    memcpy(&sock->stats_snap, &sock->stats, sizeof(struct net_socket_stats));
    sock->stats_snap.pending = true;

    if (STATS_Async(net_socket_stats_print, sock) < 0)
        sock->stats_snap.pending = false;
}

void tsn_stats_dump(struct tsn_task *task)
{
    int i;

    tsn_task_stats_dump(task);

    for (i = 0; i < task->params->num_rx_socket; i++)
        net_socket_stats_dump(&task->sock_rx[i]);

    for (i = 0; i < task->params->num_tx_socket; i++)
        net_socket_stats_dump(&task->sock_tx[i]);
}

int tsn_net_receive_sock(struct net_socket *sock)
{
    struct tsn_task *task = container_of(sock, struct tsn_task, sock_rx[sock->id]);
    int len;
    int status;

    len = genavb_socket_rx(sock->genavb_rx, sock->buf, task->params->rx_buf_size, &sock->ts);
    if (len > 0) {
        status = NET_OK;
        sock->len = len;
        sock->stats.frames++;
    } else if (len == -GENAVB_ERR_SOCKET_AGAIN) {
        status = NET_NO_FRAME;
    } else {
        status = NET_ERR;
        sock->stats.err++;
    }

    return status;
}

int tsn_net_transmit_sock(struct net_socket *sock)
{
    int rc;
    int status;

    rc = genavb_socket_tx(sock->genavb_tx, sock->buf, sock->len);
    if (rc == GENAVB_SUCCESS) {
        status = NET_OK;
        sock->stats.frames++;
    } else {
        status = NET_ERR;
        sock->stats.err++;
    }

    return status;
}

int tsn_net_receive_set_cb(struct net_socket *sock, void (*net_rx_cb)(void *))
{
    int rc;

    rc = genavb_socket_rx_set_callback(sock->genavb_rx, net_rx_cb, sock);
    if (rc != GENAVB_SUCCESS)
        return -1;

    return 0;
}

int tsn_net_receive_enable_cb(struct net_socket *sock)
{
    int rc;

    rc = genavb_socket_rx_enable_callback(sock->genavb_rx);
    if (rc != GENAVB_SUCCESS)
        return -1;

    return 0;
}

/*
 * Returns the complete transmit time including MAC framing and physical
 * layer overhead (802.3).
 * \return transmit time in nanoseconds
 * \param frame_size frame size without any framing
 * \param speed_mbps link speed in Mbps
 */
static unsigned int frame_tx_time_ns(unsigned int frame_size, int speed_mbps)
{
    unsigned int eth_size;

    eth_size = sizeof(struct eth_hdr) + frame_size + ETHER_FCS;

    if (eth_size < ETHER_MIN_FRAME_SIZE)
        eth_size = ETHER_MIN_FRAME_SIZE;

    eth_size += ETHER_IFG + ETHER_PREAMBLE;

    return (((1000 / speed_mbps) * eth_size * 8) + ST_TX_TIME_MARGIN);
}

static void tsn_net_st_config_enable(struct tsn_task *task)
{
    struct genavb_st_config config;
    struct genavb_st_gate_control_entry gate_list[ST_LIST_LEN];
    struct net_address *addr = &task->params->tx_params[0].addr;
    unsigned int cycle_time = task->params->task_period_ns;
    uint8_t iso_traffic_prio = addr->priority;
    const uint8_t *map;
    uint8_t tclass;
    unsigned int iso_tx_time = frame_tx_time_ns(task->params->tx_buf_size, 1000) * ST_TX_TIME_FACTOR;
    int i;

    map = priority_to_traffic_class_map(CFG_TRAFFIC_CLASS_MAX, CFG_SR_CLASS_MAX);
    if (!map) {
        ERR("priority_to_traffic_class_map() error\n");
        return;
    } else {
        tclass = map[iso_traffic_prio];
    }

    gate_list[0].gate_states = 1 << tclass;

    if (task->params->use_fp) {
        gate_list[0].operation = GENAVB_ST_SET_AND_HOLD_MAC;

        /*
         * Keep preemptable queues always open.
         * Match configuration done in tsn_net_fp_config_enable().
         */
        for (i = 0; i < tclass; i++)
            gate_list[0].gate_states |= 1 << i;
    } else {
        gate_list[0].operation = GENAVB_ST_SET_GATE_STATES;
    }

    gate_list[0].time_interval = iso_tx_time;

    if (task->params->use_fp)
        gate_list[1].operation = GENAVB_ST_SET_AND_RELEASE_MAC;
    else
        gate_list[1].operation = GENAVB_ST_SET_GATE_STATES;

    gate_list[1].gate_states = ~(1 << tclass);
    gate_list[1].time_interval = cycle_time - iso_tx_time;

    /* Scheduled traffic will start when (base_time + N * cycle_time) > now */
    config.enable = 1;
    config.base_time = task->params->task_period_offset_ns + task->params->sched_traffic_offset;
    config.cycle_time_p = cycle_time;
    config.cycle_time_q = NSECS_PER_SEC;
    config.cycle_time_ext = 0;
    config.list_length = ST_LIST_LEN;
    config.control_list = gate_list;

    if (genavb_st_set_admin_config(addr->port, task->params->clk_id, &config) < 0)
        ERR("genavb_st_set_admin_config() error\n");
    else
        INF("scheduled traffic config enabled\n");
}

static void tsn_net_st_config_disable(struct tsn_task *task)
{
    struct genavb_st_config config;
    struct net_address *addr = &task->params->tx_params[0].addr;

    config.enable = 0;

    if (genavb_st_set_admin_config(addr->port, task->params->clk_id, &config) < 0)
        ERR("genavb_st_set_admin_config() error\n");
    else
        INF("scheduled traffic config disabled\n");
}

static void tsn_net_st_oper_config_print(struct tsn_task *task)
{
    int i;
    struct genavb_st_config config;
    struct genavb_st_gate_control_entry gate_list[ST_LIST_LEN];
    struct net_address *addr = &task->params->tx_params[0].addr;

    config.control_list = gate_list;

    if (genavb_st_get_config(addr->port, GENAVB_ST_OPER, &config, ST_LIST_LEN) < 0) {
        ERR("genavb_st_get_config() error\n");
        return;
    }

    INF("base time   : %llu\n", config.base_time);
    INF("cycle time  : %u / %u\n", config.cycle_time_p, config.cycle_time_q);
    INF("ext time    : %u\n", config.cycle_time_ext);

    for (i = 0; i < config.list_length; i++)
        INF("%u op: %u, interval: %u, gates: %b\n",
            i, gate_list[i].operation, gate_list[i].time_interval, gate_list[i].gate_states);
}

static void tsn_net_fp_config_enable(struct tsn_task *task)
{
    struct genavb_fp_config config;
    struct net_address *addr = &task->params->tx_params[0].addr;
    const uint8_t *map;
    uint8_t tclass;
    int i;

    map = priority_to_traffic_class_map(CFG_TRAFFIC_CLASS_MAX, CFG_SR_CLASS_MAX);
    if (!map) {
        ERR("priority_to_traffic_class_map() error\n");
        return;
    } else {
        tclass = map[addr->priority];
    }

    for (i = 0; i < QOS_PRIORITY_MAX; i++) {
        if (map[i] >= tclass)
            config.u.cfg_802_1Q.admin_status[i] = GENAVB_FP_ADMIN_STATUS_EXPRESS;
        else
            config.u.cfg_802_1Q.admin_status[i] = GENAVB_FP_ADMIN_STATUS_PREEMPTABLE;
    }

    if (genavb_fp_set(addr->port, GENAVB_FP_CONFIG_802_1Q, &config) < 0) {
        ERR("genavb_fp_set(802.1Q) error\n");
        return;
    }

    config.u.cfg_802_3.enable_tx = 1;
    config.u.cfg_802_3.verify_disable_tx = 0;
    config.u.cfg_802_3.verify_time = 100;
    config.u.cfg_802_3.add_frag_size = 0;

    if (genavb_fp_set(addr->port, GENAVB_FP_CONFIG_802_3, &config) < 0) {
        ERR("genavb_fp_set(802.3) error\n");
        return;
    }
}

static void tsn_net_fp_print(struct tsn_task *task)
{
    struct genavb_fp_config config;
    struct net_address *addr = &task->params->tx_params[0].addr;
    char status[QOS_PRIORITY_MAX * 2 + 1];
    int i, off = 0;

    if (genavb_fp_get(addr->port, GENAVB_FP_CONFIG_802_1Q, &config) < 0) {
        ERR("genavb_fp_get(802.1Q) error\n");
    } else {
        for (i = 0; i < QOS_PRIORITY_MAX; i++)
            off += snprintf(status + off, 3, "%s", config.u.cfg_802_1Q.admin_status[i] == GENAVB_FP_ADMIN_STATUS_EXPRESS ? " E" : " P");

        INF("admin status      :%s\n", status);
        INF("preemption active : %u\n", config.u.cfg_802_1Q.preemption_active);
        INF("hold request      : %u\n", config.u.cfg_802_1Q.hold_request);
    }

    if (genavb_fp_get(addr->port, GENAVB_FP_CONFIG_802_3, &config) < 0) {
        ERR("genavb_fp_get(802.3) error\n");
    } else {
        INF("status verify     : %u\n", config.u.cfg_802_3.status_verify);
        INF("enable tx         : %u\n", config.u.cfg_802_3.enable_tx);
        INF("verify disable tx : %u\n", config.u.cfg_802_3.verify_disable_tx);
        INF("verify time       : %u\n", config.u.cfg_802_3.verify_time);
        INF("add frag size     : %u\n", config.u.cfg_802_3.add_frag_size);
    }
}

int tsn_task_start(struct tsn_task *task)
{
    uint64_t now, start_time;

    if (!task->timer)
        goto err;

    if (genavb_clock_gettime64(task->params->clk_id, &now) != GENAVB_SUCCESS) {
        ERR("genavb_clock_gettime64() error\n");
        goto err;
    }

    /* Start time = rounded up second + 1 second */
    start_time = ((now + NSECS_PER_SEC / 2) / NSECS_PER_SEC + 1) * NSECS_PER_SEC;

    /* Align on cycle time and add offset */
    start_time = (start_time / task->params->task_period_ns) * task->params->task_period_ns + task->params->task_period_offset_ns;

    if (genavb_timer_start(task->timer, start_time,
                           task->params->task_period_ns, GENAVB_TIMERF_ABS) != GENAVB_SUCCESS) {
        ERR("genavb_timer_start() error\n");
        goto err;
    }

    task->sched_time = start_time + task->params->task_period_ns;

    if (task->params->use_fp)
       tsn_net_fp_config_enable(task);

    if (task->params->use_st)
        tsn_net_st_config_enable(task);

    return 0;

err:
    return -1;
}

void tsn_task_stop(struct tsn_task *task)
{
    if (task->timer) {
        genavb_timer_stop(task->timer);

        if (task->params->use_st)
            tsn_net_st_config_disable(task);
    }
}

#ifdef SRP_RESERVATION
static struct genavb_control_handle *s_msrp_handle = NULL;

static uint8_t tsn_stream_id[8] = {0xaa, 0xaa, 0xaa, 0xaa, 0xbb, 0xbb, 0xbb, 0x00};

static int msrp_init(struct genavb_handle *s_avb_handle)
{
    int genavb_result;
    int rc;

    genavb_result = genavb_control_open(s_avb_handle, &s_msrp_handle, GENAVB_CTRL_MSRP);
    if (genavb_result != GENAVB_SUCCESS) {
        ERR("avb_control_open() failed: %s\n", genavb_strerror(genavb_result));
        rc = -1;
        goto err_control_open;
    }

    return 0;

err_control_open:
    return rc;
}

static int msrp_exit(void)
{
    genavb_control_close(s_msrp_handle);

    s_msrp_handle = NULL;

    return 0;
}

static int tsn_net_rx_srp_register(struct genavb_socket_rx_params *params)
{
    struct genavb_msg_listener_register listener_register;
    struct genavb_msg_listener_response listener_response;
    struct net_address *addr = &params->addr;
    unsigned int msg_type, msg_len;
    int rc;

    listener_register.port = addr->port;
    memcpy(listener_register.stream_id, tsn_stream_id, 8);
    listener_register.stream_id[7] = addr->u.l2.dst_mac[5];

    INF("stream_params: %p\n", listener_register.stream_id);

    msg_type = GENAVB_MSG_LISTENER_REGISTER;
    msg_len = sizeof(listener_response);
    rc = genavb_control_send_sync(s_msrp_handle, (genavb_msg_type_t *)&msg_type, &listener_register, sizeof(listener_register), &listener_response, &msg_len, 1000);
    if ((rc != GENAVB_SUCCESS) || (msg_type != GENAVB_MSG_LISTENER_RESPONSE) || (listener_response.status != GENAVB_SUCCESS)) {
        ERR(STREAM_STR_FMT " failed: %s\n", STREAM_STR(listener_register.stream_id), genavb_strerror(rc));
        return -1;
    }

    return 0;
}

static int tsn_net_rx_srp_deregister(struct genavb_socket_rx_params *params)
{
    struct genavb_msg_listener_deregister listener_deregister;
    struct genavb_msg_listener_response listener_response;
    struct net_address *addr = &params->addr;
    unsigned int msg_type, msg_len;
    int rc;

    listener_deregister.port = addr->port;
    memcpy(listener_deregister.stream_id, tsn_stream_id, 8);
    listener_deregister.stream_id[7] = addr->u.l2.dst_mac[5];

    INF("stream_params: %p\n", listener_deregister.stream_id);

    msg_type = GENAVB_MSG_LISTENER_DEREGISTER;
    msg_len = sizeof(listener_response);
    rc = genavb_control_send_sync(s_msrp_handle, (genavb_msg_type_t *)&msg_type, &listener_deregister, sizeof(listener_deregister), &listener_response, &msg_len, 1000);
    if ((rc != GENAVB_SUCCESS) || (msg_type != GENAVB_MSG_LISTENER_RESPONSE) || (listener_response.status != GENAVB_SUCCESS)) {
        ERR(STREAM_STR_FMT " failed: %s\n", STREAM_STR(listener_deregister.stream_id), genavb_strerror(rc));
        return -1;
    }

    return 0;
}

static int tsn_net_tx_srp_register(struct genavb_socket_tx_params *params)
{
    struct genavb_msg_talker_register talker_register;
    struct genavb_msg_talker_response talker_response;
    struct net_address *addr = &params->addr;
    unsigned int msg_type, msg_len;
    int rc;

    talker_register.port = addr->port;
    memcpy(talker_register.stream_id, tsn_stream_id, 8);
    talker_register.stream_id[7] = addr->u.l2.dst_mac[5];

    talker_register.params.stream_class = SR_CLASS_A;
    memcpy(talker_register.params.destination_address, addr->u.l2.dst_mac, 6);
    talker_register.params.vlan_id = ntohs(addr->vlan_id);

    talker_register.params.max_frame_size = 200;
    talker_register.params.max_interval_frames = 1;
    talker_register.params.accumulated_latency = 0;
    talker_register.params.rank = NORMAL;

    msg_type = GENAVB_MSG_TALKER_REGISTER;
    msg_len = sizeof(talker_response);

    rc = genavb_control_send_sync(s_msrp_handle, (genavb_msg_type_t *)&msg_type, &talker_register, sizeof(talker_register), &talker_response, &msg_len, 1000);
    if ((rc != GENAVB_SUCCESS) || (msg_type != GENAVB_MSG_TALKER_RESPONSE) || (talker_response.status != GENAVB_SUCCESS)) {
        ERR(STREAM_STR_FMT " failed: %s\n", STREAM_STR(talker_register.stream_id), genavb_strerror(rc));
        return -1;
    }

    return 0;
}

static int tsn_net_tx_srp_deregister(struct genavb_socket_tx_params *params)
{
    struct genavb_msg_talker_deregister talker_deregister;
    struct genavb_msg_talker_response talker_response;
    struct net_address *addr = &params->addr;
    unsigned int msg_type, msg_len;
    int rc;

    talker_deregister.port = addr->port;
    memcpy(talker_deregister.stream_id, tsn_stream_id, 8);
    talker_deregister.stream_id[7] = addr->u.l2.dst_mac[5];

    msg_type = GENAVB_MSG_TALKER_DEREGISTER;
    msg_len = sizeof(talker_response);
    rc = genavb_control_send_sync(s_msrp_handle, (genavb_msg_type_t *)&msg_type, &talker_deregister, sizeof(talker_deregister), &talker_response, &msg_len, 1000);
    if ((rc != GENAVB_SUCCESS) || (msg_type != GENAVB_MSG_TALKER_RESPONSE) || (talker_response.status != GENAVB_SUCCESS)) {
        ERR(STREAM_STR_FMT " failed: %s\n", STREAM_STR(talker_deregister.stream_id), genavb_strerror(rc));

        return -1;
    }

    return 0;
}
#endif

static int tsn_task_net_init(struct tsn_task *task)
{
    int i, j, k;
    struct net_socket *sock;

#ifdef SRP_RESERVATION
    if (msrp_init(get_genavb_handle()) < 0)
        return -1;
#endif

    for (i = 0; i < task->params->num_rx_socket; i++) {
        sock = &task->sock_rx[i];
        sock->id = i;
        sock->dir = RX;

        if (genavb_socket_rx_open(&sock->genavb_rx, GENAVB_SOCKF_NONBLOCK,
                                  &task->params->rx_params[i]) != GENAVB_SUCCESS) {
            ERR("genavb_socket_rx_open error\n");
            goto close_sock_rx;
        }

        sock->buf = rtos_malloc(task->params->rx_buf_size);
        if (!sock->buf) {
            genavb_socket_rx_close(sock->genavb_rx);
            ERR("error allocating rx_buff\n");
            goto close_sock_rx;
        }

#ifdef SRP_RESERVATION
        tsn_net_rx_srp_register(&task->params->rx_params[i]);
#endif
    }

    for (j = 0; j < task->params->num_tx_socket; j++) {
        sock = &task->sock_tx[j];
        sock->id = j;
        sock->dir = TX;

        if (genavb_socket_tx_open(&sock->genavb_tx, 0, &task->params->tx_params[j]) != GENAVB_SUCCESS) {
            ERR("genavb_socket_tx_open error\n");
            goto close_sock_tx;
        }

        sock->buf = rtos_malloc(task->params->tx_buf_size);
        if (!sock->buf) {
            genavb_socket_tx_close(sock->genavb_tx);
            ERR("error allocating tx_buff\n");
            goto close_sock_tx;
        }

#ifdef SRP_RESERVATION
        tsn_net_tx_srp_register(&task->params->tx_params[j]);
#endif
    }

#ifdef SRP_RESERVATION
    msrp_exit();
#endif

    return 0;

close_sock_tx:
    for (k = 0; k < j; k++) {
        sock = &task->sock_tx[k];
#ifdef SRP_RESERVATION
        tsn_net_tx_srp_deregister(&task->params->tx_params[k]);
#endif
        rtos_free(sock->buf);
        genavb_socket_tx_close(sock->genavb_tx);
    }

close_sock_rx:
    for (k = 0; k < i; k++) {
        sock = &task->sock_rx[k];
#ifdef SRP_RESERVATION
        tsn_net_rx_srp_deregister(&task->params->rx_params[k]);
#endif
        rtos_free(sock->buf);
        genavb_socket_rx_close(sock->genavb_rx);
    }

    return -1;
}

static void tsn_task_net_exit(struct tsn_task *task)
{
    int i;
    struct net_socket *sock;

    for (i = 0; i < task->params->num_rx_socket; i++) {
        sock = &task->sock_rx[i];
#ifdef SRP_RESERVATION
        tsn_net_rx_srp_deregister(&task->params->rx_params[i]);
#endif
        rtos_free(sock->buf);
        genavb_socket_rx_close(sock->genavb_rx);
    }

    for (i = 0; i < task->params->num_tx_socket; i++) {
        sock = &task->sock_tx[i];
#ifdef SRP_RESERVATION
        tsn_net_tx_srp_deregister(&task->params->tx_params[i]);
#endif
        rtos_free(sock->buf);
        genavb_socket_tx_close(sock->genavb_tx);
    }
}

int tsn_task_register(struct tsn_task **task, struct tsn_task_params *params,
                      int id, void (*main_loop)(void *), void *ctx,
                      void (*timer_callback)(void *, int))
{
    char task_name[20] = {0, };

    *task = rtos_malloc(sizeof(struct tsn_task));
    if (!(*task))
        goto err;

    memset(*task, 0, sizeof(struct tsn_task));

    (*task)->id = id;
    (*task)->params = params;
    (*task)->ctx = ctx;

    snprintf(task_name, 19, "tsn task%1d", (*task)->id);
    task_name[19] = '\0';

    if (tsn_task_net_init(*task) < 0) {
        ERR("tsn_task_net_init error\n");
        goto err_free;
    }

    tsn_task_stats_init(*task);

    if (main_loop) {
        if (rtos_thread_create(&(*task)->thread, params->priority, 0, params->stack_depth, task_name, main_loop, ctx) < 0) {
            ERR("xTaskCreate failed\n\r");
            goto net_exit;
        }
    }

    if (timer_callback) {
        if (genavb_timer_create(&(*task)->timer, params->clk_id, 0) != GENAVB_SUCCESS) {
            ERR("genavb_timer_create() error\n");
            goto task_delete;
        }

        if (genavb_timer_set_callback((*task)->timer, timer_callback, *task) != GENAVB_SUCCESS) {
            ERR("genavb_timer_create() error\n");
            goto timer_destroy;
        }
    } else {
        (*task)->timer = NULL;
    }

    return 0;

timer_destroy:
    if ((*task)->timer)
        genavb_timer_destroy((*task)->timer);

task_delete:
    if (main_loop)
        rtos_thread_abort(&(*task)->thread);

net_exit:
    tsn_task_net_exit(*task);

err_free:
    rtos_free(*task);

err:
    return -1;
}

void tsn_task_unregister(struct tsn_task **task)
{
    /* timer_destroy */
    if ((*task)->timer)
        genavb_timer_destroy((*task)->timer);

    /* task_delete */
    rtos_thread_abort(&(*task)->thread);

    /* net_exit */
    tsn_task_net_exit(*task);

    /* err_free */
    rtos_free(*task);
}
