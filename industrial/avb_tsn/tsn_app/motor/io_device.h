/*
 * Copyright 2019, 2021, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IO_DEVICE_H_
#define _IO_DEVICE_H_

#include "motor_control_api.h"
#include "motor_control.h"
#include "monitoring_stats.h"
#include "rtos_abstraction_layer.h"

typedef enum {
    WAIT_FOR_INPUT,
    INIT,
    STAY_INDEX,
    REMOTE_CONTROL,
    FREERUNNING,
    MOTOR_FAULT,
} sm_io_device_state_t;

struct stats_io_device {
    bool pending;
    sm_io_device_state_t state;
    uint32_t err_msg_id;
    uint32_t err_cmd_recv;
    uint32_t err_invalid_len_received;
};

struct stats_motor {
    bool pending;
    uint8_t state;
    float iq_req;
    float pos_real;
    float speed_real;
    uint32_t last_index;
    uint32_t missed_slow_loop;
    uint32_t counter_rev_jumps;
};

struct motor_controlled {
    uint16_t motor_id;
    float iq_req;
    bool iq_received;
    float motor_offset;
    struct tsn_motor *motor;
    struct stats_motor stats;
    struct stats_motor stats_snap;
};

struct io_device_ctx {
    uint16_t num_motors;
    bool controller_local;
    struct motor_controlled motors_controlled[NB_MOTORS_MAX];
    struct cyclic_task *c_task;
    sm_io_device_state_t state;
    uint16_t status;
    struct stats_io_device stats;
    struct stats_io_device stats_snap;
    rtos_mqueue_t *event_queue;
    uint32_t stay_index_delay;
    bool offset_reached;
    uint16_t control_action;
    struct monitoring_stats_ctx *monitoring_stats_ctx;
    struct monitoring_msg msg;
    bool msg_pending;
};

int io_device_init(struct io_device_ctx *ctx, struct cyclic_task *c_task, uint16_t nb_motors, bool motor_local);
void io_device_set_motor_offset(struct io_device_ctx *ctx, int motor_id, float offset);
void io_device_net_receive(void *data, int msg_id, int src_id, void *buf, int len);

#endif /* _IO_DEVICE_H_ */
