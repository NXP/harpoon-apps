/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "log.h"
#include "genavb.h"
#include "storage.h"
#include "rtos_abstraction_layer.h"

#include "genavb/config.h"
#include "genavb/genavb.h"
#include "genavb/log.h"
#include "genavb/stats.h"
#include "genavb/timer.h"

static struct genavb_handle *s_genavb_handle = NULL;
static struct port_stats port_stats[CFG_EP_DEFAULT_NUM_PORTS];

struct genavb_handle *get_genavb_handle(void)
{
    return s_genavb_handle;
}

static int timer_pps_start(struct gavb_pps *pps)
{
    uint64_t now, start_time;
    int rc;

    rc = genavb_clock_gettime64(pps->clk_id, &now);
    if (rc != GENAVB_SUCCESS) {
        ERR("genavb_clock_gettime64() error %d \n", rc);
        goto err;
    }

    /* Start time = rounded up second + 1 second */
    start_time = ((now + NSECS_PER_SEC / 2) / NSECS_PER_SEC + 1) * NSECS_PER_SEC;

    rc = genavb_timer_start(pps->t, start_time, NSECS_PER_SEC, GENAVB_TIMERF_PPS | GENAVB_TIMERF_ABS);
    if (rc != GENAVB_SUCCESS) {
        ERR("genavb_timer_start error %d \n", rc);
        goto err;
    }

    return 0;

err:
    return -1;
}

static void timer_callback(void *data, int count)
{
    struct gavb_pps *pps = (struct gavb_pps *)data;

    /* Handle discontinuities */
    if (count < 0) {
        timer_pps_start(pps);
        INF("discontinuity : callback_counter %d \n", count);
    }
}

int gavb_pps_init(struct gavb_pps *pps, genavb_clock_id_t clk_id)
{
    int rc;

    pps->clk_id = clk_id;

    rc = genavb_timer_create(&pps->t, pps->clk_id, GENAVB_TIMERF_PPS);
    if (rc != GENAVB_SUCCESS) {
        ERR("genavb_timer_create error %d \n", rc);
        goto err;
    }

    rc = genavb_timer_set_callback(pps->t, timer_callback, pps);
    if (rc != GENAVB_SUCCESS) {
        ERR("genavb_timer_set_callback error %d \n", rc);
        goto err_destroy;
    }

    rc = timer_pps_start(pps);
    if (rc != 0) {
        ERR("timer_pps_start error %d \n", rc);
        goto err_destroy;
    }

    INF("success, clk_id: %u\n", clk_id);
    return 0;

err_destroy:
    genavb_timer_destroy(pps->t);

err:
    return -1;
}

void gavb_pps_exit(struct gavb_pps *pps)
{
    genavb_timer_destroy(pps->t);
}

static char *domain_cfg_param_file(uint8_t instance, char *param, char *buf, size_t size)
{
    if (!instance)
        snprintf(buf, size, "%s", param);
    else
        snprintf(buf, size, "domain%u/%s", instance, param);

    return buf;
}

static char *port_cfg_param_file(uint8_t port, char *param, char *buf, size_t size)
{
    snprintf(buf, size, "port%u/%s", port, param);

    return buf;
}

int gavb_stack_init(void)
{
    struct genavb_config *genavb_config;
    char buf[128];
    int i;
    int rc = 0;

    genavb_config = rtos_malloc(sizeof(struct genavb_config));

    if (!genavb_config) {
        rc = -1;
        goto exit;
    }

    if (s_genavb_handle) {
        rc = 0;
        goto exit;
    }

    genavb_get_default_config(genavb_config);

    if (storage_cd("/fgptp") == 0) {

        /* Read general parameters */
        storage_read_uint("force_2011", &genavb_config->fgptp_config.force_2011);

        /* Read per-domain parameters */
        for (i = 0; i < CFG_MAX_GPTP_DOMAINS; i++) {

            if (i != 0)
                storage_read_int(domain_cfg_param_file(i, "domain_number", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].domain_number);

            storage_read_u8(domain_cfg_param_file(i, "gmCapable", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].gmCapable);
            storage_read_u8(domain_cfg_param_file(i, "priority1", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].priority1);
            storage_read_u8(domain_cfg_param_file(i, "priority2", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].priority2);
            storage_read_u8(domain_cfg_param_file(i, "clockClass", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].clockClass);
            storage_read_u8(domain_cfg_param_file(i, "clockAccuracy", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].clockAccuracy);
            storage_read_u16(domain_cfg_param_file(i, "offsetScaledLogVariance", buf, sizeof(buf)), &genavb_config->fgptp_config.domain_cfg[i].offsetScaledLogVariance);
        }

        /* Read per-port parameters */
        for (i = 0; i < CFG_MAX_NUM_PORT; i++) {
            storage_read_int(port_cfg_param_file(i, "rxDelayCompensation", buf, sizeof(buf)), &genavb_config->fgptp_config.port_cfg[i].rxDelayCompensation);
            storage_read_int(port_cfg_param_file(i, "txDelayCompensation", buf, sizeof(buf)), &genavb_config->fgptp_config.port_cfg[i].txDelayCompensation);
        }
    }

    if (storage_cd("/avdecc") == 0) {
        int btb_mode = 0;
        uint64_t talker_entity_id = 0;

        storage_read_int("btb_mode", &btb_mode);
        if (btb_mode) {
            INF("BTB mode\n");
            genavb_config->avdecc_config.entity_cfg[0].flags |= AVDECC_FAST_CONNECT_MODE | AVDECC_FAST_CONNECT_BTB;
            genavb_config->avdecc_config.entity_cfg[0].channel_waitmask |= AVDECC_WAITMASK_MEDIA_STACK;
        }

        if (!storage_read_u64("talker_id", &talker_entity_id)) {
            INF("talker_entity_id 0x%016"PRIx64"\n", talker_entity_id);
            genavb_config->avdecc_config.entity_cfg[0].talker_entity_id[0] = talker_entity_id;
            genavb_config->avdecc_config.entity_cfg[0].talker_entity_id_n = 1;
            genavb_config->avdecc_config.entity_cfg[0].talker_unique_id[0] = 0;
            genavb_config->avdecc_config.entity_cfg[0].talker_unique_id_n = 1;
            genavb_config->avdecc_config.entity_cfg[0].listener_unique_id[0] = 0;
            genavb_config->avdecc_config.entity_cfg[0].listener_unique_id_n = 1;
        }
    }
    storage_cd("/");

    genavb_set_config(genavb_config);

    if ((rc = genavb_init(&s_genavb_handle, 0)) != GENAVB_SUCCESS) {
        s_genavb_handle = NULL;
        ERR("genavb_init() failed: %s\n", genavb_strerror(rc));
        rc = -1;
        goto exit;
    }

exit:
    if (genavb_config)
        rtos_free(genavb_config);

    return rc;
}

int gavb_stack_exit(void)
{
    genavb_exit(s_genavb_handle);

    s_genavb_handle = NULL;

    return 0;
}

int gavb_log_level(char *component_str, char *level_str)
{
    genavb_log_component_id_t component;
    genavb_log_level_t level;
    int all = 0;

    if (!strcmp(level_str, "crit"))
        level = GENAVB_LOG_LEVEL_CRIT;
    else if (!strcmp(level_str, "err"))
        level = GENAVB_LOG_LEVEL_ERR;
    else if (!strcmp(level_str, "init"))
        level = GENAVB_LOG_LEVEL_INIT;
    else if (!strcmp(level_str, "info"))
        level = GENAVB_LOG_LEVEL_INFO;
    else if (!strcmp(level_str, "dbg"))
        level = GENAVB_LOG_LEVEL_DEBUG;
    else
        return -1;

    if (!strcmp(component_str, "avtp"))
        component = GENAVB_LOG_COMPONENT_ID_AVTP;
    else if (!strcmp(component_str, "avdecc"))
        component = GENAVB_LOG_COMPONENT_ID_AVDECC;
    else if (!strcmp(component_str, "srp"))
        component = GENAVB_LOG_COMPONENT_ID_SRP;
    else if (!strcmp(component_str, "maap"))
        component = GENAVB_LOG_COMPONENT_ID_MAAP;
    else if (!strcmp(component_str, "common"))
        component = GENAVB_LOG_COMPONENT_ID_COMMON;
    else if (!strcmp(component_str, "os"))
        component = GENAVB_LOG_COMPONENT_ID_OS;
    else if (!strcmp(component_str, "fgptp"))
        component = GENAVB_LOG_COMPONENT_ID_GPTP;
    else if (!strcmp(component_str, "api"))
        component = GENAVB_LOG_COMPONENT_ID_API;
    else if (!strcmp(component_str, "mgmt"))
        component = GENAVB_LOG_COMPONENT_ID_MGMT;
    else if (!strcmp(component_str, "all"))
        all = 1;
    else
        return -1;

    if (all) {
        for (component = GENAVB_LOG_COMPONENT_ID_AVTP; component <= GENAVB_LOG_COMPONENT_ID_MGMT; component++)
            genavb_log_level_set(component, level);
    } else {
        genavb_log_level_set(component, level);
    }

    return 0;
}

int gavb_port_stats_init(unsigned int port_id)
{
    int num;
    struct port_stats *stats = &port_stats[port_id];

    num = genavb_port_stats_get_number(port_id);
    if (num < 0) {
        ERR("genavb_port_stats_get_number() error %d\n", num);
        goto err;
    }

    stats->num = num;
    stats->names = rtos_malloc(num * sizeof(char *));
    if (!stats->names) {
        ERR("rtos_malloc() failed\n");
        goto err;
    }

    if (genavb_port_stats_get_strings(port_id, stats->names, num * sizeof(char *)) != GENAVB_SUCCESS) {
        ERR("genavb_port_stats_get_strings() failed\n");
        goto err;
    }

    stats->values = rtos_malloc(num * sizeof(uint64_t));
    if (!stats->values) {
        ERR("rtos_malloc() failed\n");
        goto err;
    }

    return 0;

err:
    if (stats->names)
        rtos_free(stats->names);

    return -1;
}

void gavb_port_stats_exit(unsigned int port_id)
{
    struct port_stats *stats = &port_stats[port_id];

    if (stats->values)
        rtos_free(stats->values);

    if (stats->names)
        rtos_free(stats->names);
}

struct port_stats *gavb_port_stats_get(unsigned int port_id)
{
    struct port_stats *stats = &port_stats[port_id];

    if (!stats->names || !stats->values) {
        ERR("port stats are not initialized\n");
        goto err;
    }

    if (genavb_port_stats_get(port_id, stats->values, stats->num * sizeof(uint64_t)) < 0) {
        ERR("genavb_port_stats_get() error\n");
        goto err;
    }

    return stats;

err:
    return NULL;
}
