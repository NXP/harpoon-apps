/*
 * Copyright 2019, 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LOG_H_
#define _LOG_H_

#include "rtos_apps/log.h"
#include "rtos_apps/types.h"

#include "genavb/clock.h"

#define STREAM_STR_FMT         "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x"
#define STREAM_STR(_stream_id) (_stream_id)[0], (_stream_id)[1], (_stream_id)[2], (_stream_id)[3], (_stream_id)[4], (_stream_id)[5], (_stream_id)[6], (_stream_id)[7]

#define MAC_STR_FMT       "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_STR(_dst_mac) (_dst_mac)[0], (_dst_mac)[1], (_dst_mac)[2], (_dst_mac)[3], (_dst_mac)[4], _dst_mac[5]

void app_log_update_time(genavb_clock_id_t clk_id);
int app_log_level_set(char *level_str);

#endif /* _LOG_H */
