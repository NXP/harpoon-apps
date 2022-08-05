/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "system_config.h"
#include "tsn_tasks_config.h"
#include "board.h"

#define CONTROLLER_NUM_IO_DEVICES 1

struct system_config system_cfg = {
    .net = {
        [0] = {
            .hw_addr = {0x00, 0xBB, 0xCC, 0xDD, 0xEE, 0x12},
            .ip_addr = {192, 168, 1, 4},
            .net_mask = {255, 255, 255, 0},
            .gw_addr = {192, 168, 1, 1},
        },
#if BOARD_NUM_PORTS > 1
        [1] = {
            .hw_addr = {0x00, 0x11, 0x22, 0x22, 0x44, 0x02},
            .ip_addr = {192, 168, 30, 4},
            .net_mask = {255, 255, 255, 0},
            .gw_addr = {192, 168, 30, 1},
        },
#endif
    },
    .app = {
        .tsn_app_config = {
            .mode = NETWORK_ONLY,
            .role = CONTROLLER_0,
            .num_io_devices = CONTROLLER_NUM_IO_DEVICES,
            .control_strategy = SYNCHRONIZED,
            .use_st = 1,
            .use_fp = 0,
            .cmd_client = 0,
            .period_ns = APP_PERIOD_DEFAULT,
        },
    },
};
