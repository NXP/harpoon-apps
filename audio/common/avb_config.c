/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_config.h"
#include "clock_domain.h"
#include "board.h"

struct system_config system_cfg = {
    .net = {
        [0] = {
            .hw_addr = {0x00, 0xBB, 0xCC, 0xDD, 0xEE, 0x10},
        },
#if BOARD_NUM_PORTS > 1
        [1] = {
            .hw_addr = {0x00, 0x11, 0x22, 0x22, 0x44, 0x00},
        },
#endif
    },
    .app = {
        .avb_app_config = {
            .mclock_role = MEDIA_CLOCK_SLAVE,
        },
    },
};
