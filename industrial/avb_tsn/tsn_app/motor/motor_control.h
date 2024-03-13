/*
 * Copyright 2019, 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MOTOR_CONTROL_H_
#define _MOTOR_CONTROL_H_

#include <stdint.h>

#define NB_IO_DEVICE_MAX  2
#define NB_MOTORS_MAX 1

#define IO_DEVICE_STATUS_ERR_FAULT   (1 << 0)
#define IO_DEVICE_STATUS_ERR_NETWORK (1 << 1)

static inline void io_device_status_set_error_network(uint16_t *status)
{
    *status |= IO_DEVICE_STATUS_ERR_NETWORK;
}

static inline void io_device_status_set_error_fault(uint16_t *status)
{
    *status |= IO_DEVICE_STATUS_ERR_FAULT;
}

static inline void io_device_status_clear_error_network(uint16_t *status)
{
    *status &= ~IO_DEVICE_STATUS_ERR_NETWORK;
}

static inline void io_device_status_clear_error_fault(uint16_t *status)
{
    *status &= ~IO_DEVICE_STATUS_ERR_FAULT;
}

enum event_motor {
    BUTTON_PRESSED,
};

enum controller_action {
    HOLD = 0,
    APPLY_IQ = 1,
};

struct motor_feedback {
    uint16_t motor_id;
    float pos;
    float speed;
    float iq_meas;
    float id_meas;
    float uq_applied;
    float dc_bus;
    float cur_a;
    float cur_b;
    float cur_c;
};

struct msg_feedback {
    uint16_t status;
    uint16_t num_msg;
    struct motor_feedback msg_array[NB_MOTORS_MAX];
};

struct motor_set_iq {
    uint16_t io_device_id;
    uint16_t motor_id;
    float iq_req;
};

struct msg_set_iq {
    uint16_t action;
    uint16_t num_msg;
    struct motor_set_iq msg_array[NB_MOTORS_MAX * NB_IO_DEVICE_MAX];
};

#endif /* _MOTOR_CONTROL_H_ */
