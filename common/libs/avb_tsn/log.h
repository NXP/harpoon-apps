/*
 * Copyright 2019, 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "rtos_abstraction_layer.h"

#include "types.h"
#include "genavb/clock.h"

extern uint64_t app_log_time_s;
extern unsigned int app_log_level;

void app_log_update_time(genavb_clock_id_t clk_id);
int app_log_level_set(char *level_str);

#define VERBOSE_NONE  (0)
#define VERBOSE_ERROR (1)
#define VERBOSE_INFO  (2)
#define VERBOSE_DEBUG (3)

#ifndef PRINT_LEVEL
#define PRINT_LEVEL VERBOSE_DEBUG
#endif /* PRINT_LEVEL */

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define DBG_OUT_BASE(fmt, args...) rtos_printf("DBG  %11llu app %-25s: %-25s: " fmt, app_log_time_s, __FILENAME__, __FUNCTION__, ##args)
#define INF_OUT_BASE(fmt, args...) rtos_printf("INFO %11llu app %-25s: " fmt, app_log_time_s, __FUNCTION__, ##args)
#define ERR_OUT_BASE(fmt, args...) rtos_printf("ERR  %11llu app %-25s: " fmt, app_log_time_s, __FUNCTION__, ##args)

#if PRINT_LEVEL >= VERBOSE_DEBUG
#define DBG_OUT(fmt, args...)               \
    do {                                    \
        if (app_log_level >= VERBOSE_DEBUG) \
            DBG_OUT_BASE(fmt, ##args);      \
    } while (0)
#else
#define DBG_OUT(fmt, args...)
#endif /* PRINT_LEVEL >= VERBOSE_DEBUG */

#if PRINT_LEVEL >= VERBOSE_INFO
#define INF_OUT(fmt, args...)              \
    do {                                   \
        if (app_log_level >= VERBOSE_INFO) \
            INF_OUT_BASE(fmt, ##args);     \
    } while (0)
#else
#define INF_OUT(fmt, args...)
#endif /* PRINT_LEVEL >= VERBOSE_INFO */

#if PRINT_LEVEL >= VERBOSE_ERROR
#define ERR_OUT(fmt, args...)               \
    do {                                    \
        if (app_log_level >= VERBOSE_ERROR) \
            ERR_OUT_BASE(fmt, ##args);      \
    } while (0)
#else
#define ERR_OUT(fmt, args...)
#endif /* PRINT_LEVEL >= VERBOSE_ERROR */

#define DBG(fmt, args...) DBG_OUT(fmt, ##args)
#define INF(fmt, args...) INF_OUT(fmt, ##args)
#define ERR(fmt, args...) ERR_OUT(fmt, ##args)

#define STREAM_STR_FMT         "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x"
#define STREAM_STR(_stream_id) _stream_id[0], _stream_id[1], _stream_id[2], _stream_id[3], _stream_id[4], _stream_id[5], _stream_id[6], _stream_id[7]

#define MAC_STR_FMT       "%02x-%02x-%02x-%02x-%02x-%02x"
#define MAC_STR(_dst_mac) _dst_mac[0], _dst_mac[1], _dst_mac[2], _dst_mac[3], _dst_mac[4], _dst_mac[5]

#endif /* _LOG_H */
