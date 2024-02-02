/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TSN_TASKS_CONFIG_H_
#define _TSN_TASKS_CONFIG_H_

#include "rtos_abstraction_layer.h"

#include "genavb/tsn.h"
#include "genavb/qos.h"

#define MAX_PEERS          2
#define MAX_TASK_CONFIGS   4
#define MAX_TSN_STREAMS    8
#define ETHERTYPE_MOTOROLA 0x818D
#define VLAN_ID            2
#define PACKET_SIZE        80

#define SCHED_TRAFFIC_OFFSET 35000

#define TASK_DEFAULT_STACK_SIZE   (RTOS_MINIMAL_STACK_SIZE + 256)
#define TASK_DEFAULT_PRIORITY     (RTOS_MAX_PRIORITY - 1)
#define TASK_DEFAULT_QUEUE_LENGTH (8)

#define APP_PERIOD_DEFAULT              100000
#define APP_PERIOD_MIN                  100000
#define APP_PERIOD_MAX                  1000000000
#define NET_DELAY_OFFSET_DEFAULT        (APP_PERIOD_DEFAULT / 2)

#define APP_PERIOD_SERIAL_DEFAULT       2000000
#define NET_DELAY_OFFSET_SERIAL_DEFAULT (APP_PERIOD_SERIAL_DEFAULT / 2)

enum task_id {
    CONTROLLER_0,
    IO_DEVICE_0,
    IO_DEVICE_1,
    MAX_TASKS_ID
};

enum task_type {
    CYCLIC_CONTROLLER,
    CYCLIC_IO_DEVICE,
    ALARM_MONITOR,
    ALARM_IO_DEVICE,
};

// Supported APP_MODEs
#define MOTOR_NETWORK 0
#define MOTOR_LOCAL   1
#define NETWORK_ONLY  2
#define SERIAL        3

typedef enum control_strategies {
    SYNCHRONIZED,
    FOLLOW,
    HOLD_INDEX,
    INTERLACED,
    STOP,
} control_strategies_t;

struct tsn_stream {
    struct net_address address;
};

struct tsn_stream *tsn_conf_get_stream(int index);
struct cyclic_task *tsn_conf_get_cyclic_task(int index);
struct alarm_task *tsn_conf_get_alarm_task(int index);

#endif /* _TSN_TASKS_CONFIG_H_ */
