/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _LOG_H_
#define _LOG_H_

#include "os/stdio.h"

/** Log levels definition */
typedef enum {
  LOG_CRIT,
  LOG_ERR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG,

  LOG_LEVEL_MAX
} log_level_t;

/** Current log level configuration */
extern log_level_t log_level_config;

/** Logging macros definitions
 *
 *  Usage:
 *
 *      log(INFO, "---\n");
 *  or
 * 	    log_info("---\n");
 */
#define log(LEVEL, format, ...) \
  do { \
    if (log_level_config >= LOG_ ## LEVEL) \
         os_printf("%-4.4s: %-22.22s: " format "\r", #LEVEL, __func__, ##__VA_ARGS__); \
  } while (0)

#define log_crit(...)     log(CRIT, __VA_ARGS__)
#define log_err(...)      log(ERR, __VA_ARGS__)
#define log_warn(...)     log(WARN, __VA_ARGS__)
#define log_info(...)     log(INFO, __VA_ARGS__)
#define log_debug(...)    log(DEBUG, __VA_ARGS__)

#define log_raw(LEVEL, ...) \
  do { \
    if (log_level_config >= LOG_ ## LEVEL) \
         os_printf(__VA_ARGS__); \
  } while (0)

#define log_raw_info(...) log_raw(INFO, __VA_ARGS__)

/** Set current log level configuration */
static inline void log_level_config_set(log_level_t log_level)
{
    if (log_level < LOG_LEVEL_MAX)
        log_level_config = log_level;
}

#endif /* _LOG_H_ */
