/*
 * Copyright 2019, 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "alarm_task.h"
#include "cyclic_task.h"
#include "tsn_tasks_config.h"

struct tsn_stream tsn_streams[MAX_TSN_STREAMS] = {
    [0] = {
        .address = {
            .ptype = PTYPE_L2,
            .vlan_id = htons(VLAN_ID),
            .priority = ISOCHRONOUS_DEFAULT_PRIORITY,
            .u.l2 = {
                .dst_mac = {0x91, 0xe0, 0xf0, 0x00, 0xfe, 0x70},
                .protocol = htons(ETHERTYPE_MOTOROLA),
            },
        },
    },
    [1] = {
        .address = {
            .ptype = PTYPE_L2,
            .vlan_id = htons(VLAN_ID),
            .priority = ISOCHRONOUS_DEFAULT_PRIORITY,
            .u.l2 = {
                .dst_mac = {0x91, 0xe0, 0xf0, 0x00, 0xfe, 0x71},
                .protocol = htons(ETHERTYPE_MOTOROLA),
            },
        },
    },
    [2] = {
        .address = {
            .ptype = PTYPE_L2,
            .vlan_id = htons(VLAN_ID),
            .priority = ISOCHRONOUS_DEFAULT_PRIORITY,
            .u.l2 = {
                .dst_mac = {0x91, 0xe0, 0xf0, 0x00, 0xfe, 0x80},
                .protocol = htons(ETHERTYPE_MOTOROLA),
            },
        },
    },
    [3] = {
        .address = {
            .ptype = PTYPE_L2,
            .vlan_id = htons(VLAN_ID),
            .priority = EVENTS_DEFAULT_PRIORITY,
            .u.l2 = {
                .dst_mac = {0x91, 0xe0, 0xf0, 0x00, 0xfe, 0x90},
                .protocol = htons(ETHERTYPE_MOTOROLA),
            },
        },
    },
    [4] = {
        .address = {
            .ptype = PTYPE_L2,
            .vlan_id = htons(VLAN_ID),
            .priority = EVENTS_DEFAULT_PRIORITY,
            .u.l2 = {
                .dst_mac = {0x91, 0xe0, 0xf0, 0x00, 0xfe, 0xa0},
                .protocol = htons(ETHERTYPE_MOTOROLA),
            },
        },
    },
};

#define CYCLIC_TASK_DEFAULT_PARAMS(offset)            \
    {                                                 \
        .clk_id = GENAVB_CLOCK_GPTP_0_0,              \
        .priority = TASK_DEFAULT_PRIORITY,            \
        .stack_depth = TASK_DEFAULT_STACK_SIZE,       \
        .task_period_ns = APP_PERIOD_DEFAULT,         \
        .task_period_offset_ns = offset,              \
        .transfer_time_ns = NET_DELAY_OFFSET_DEFAULT, \
        .use_st = 1,                                  \
        .use_fp = 0,                                  \
        .sched_traffic_offset = SCHED_TRAFFIC_OFFSET, \
        .rx_buf_size = PACKET_SIZE,                   \
        .tx_buf_size = PACKET_SIZE,                   \
    }

struct cyclic_task cyclic_tasks[MAX_TASK_CONFIGS] = {
    [0] = {
        .type = CYCLIC_CONTROLLER,
        .id = CONTROLLER_0,
        .params = CYCLIC_TASK_DEFAULT_PARAMS(0),
        .num_peers = 2,
        .rx_socket = {
            [0] = {
                .peer_id = IO_DEVICE_0,
                .stream_id = 1,
            },
            [1] = {
                .peer_id = IO_DEVICE_1,
                .stream_id = 2,
            },
        },
        .tx_socket = {
            .stream_id = 0,
        },
    },
    [1] = {
        .type = CYCLIC_IO_DEVICE,
        .id = IO_DEVICE_0,
        .params = CYCLIC_TASK_DEFAULT_PARAMS(NET_DELAY_OFFSET_DEFAULT),
        .num_peers = 1,
        .rx_socket = {
            [0] = {
                .peer_id = CONTROLLER_0,
                .stream_id = 0,
            },
        },
        .tx_socket = {
            .stream_id = 1,
        },
    },
    [2] = {
        .type = CYCLIC_IO_DEVICE,
        .id = IO_DEVICE_1,
        .params = CYCLIC_TASK_DEFAULT_PARAMS(NET_DELAY_OFFSET_DEFAULT),
        .num_peers = 1,
        .rx_socket = {
            [0] = {
                .peer_id = CONTROLLER_0,
                .stream_id = 0,
            },
        },
        .tx_socket = {
            .stream_id = 2,
        },
    },
};

#define ALARM_TASK_DEFAULT_PARAMS               \
    {                                           \
        .clk_id = GENAVB_CLOCK_GPTP_0_0,        \
        .priority = TASK_DEFAULT_PRIORITY - 1,  \
        .stack_depth = TASK_DEFAULT_STACK_SIZE, \
        .rx_buf_size = PACKET_SIZE,             \
        .tx_buf_size = PACKET_SIZE,             \
    }

struct alarm_task alarm_tasks[MAX_TASK_CONFIGS] = {
    [0] = {
        .type = ALARM_MONITOR,
        .id = CONTROLLER_0,
        .params = ALARM_TASK_DEFAULT_PARAMS,
        .num_peers = 2,
        .stream_id = 4,
        .queue = {
            .length = TASK_DEFAULT_QUEUE_LENGTH,
        },
    },
    [1] = {
        .type = ALARM_IO_DEVICE,
        .id = IO_DEVICE_0,
        .params = ALARM_TASK_DEFAULT_PARAMS,
        .num_peers = 1,
        .stream_id = 4,
    },
    [2] = {
        .type = ALARM_IO_DEVICE,
        .id = IO_DEVICE_1,
        .params = ALARM_TASK_DEFAULT_PARAMS,
        .num_peers = 1,
        .stream_id = 4,
    },
};

struct tsn_stream *tsn_conf_get_stream(int index)
{
    if (index >= (sizeof(tsn_streams) / sizeof(struct tsn_stream)))
        return NULL;

    return &tsn_streams[index];
}

struct cyclic_task *tsn_conf_get_cyclic_task(int index)
{
    struct cyclic_task *c_task;

    if (index >= (sizeof(cyclic_tasks) / sizeof(struct cyclic_task)))
        return NULL;

    c_task = &cyclic_tasks[index];

    return c_task;
}

struct alarm_task *tsn_conf_get_alarm_task(int index)
{
    if (index >= (sizeof(alarm_tasks) / sizeof(struct alarm_task)))
        return NULL;

    return &alarm_tasks[index];
}
