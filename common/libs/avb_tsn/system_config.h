/*
 * Copyright 2019, 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System configuration
 */
#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

#if (CONFIG_GENAVB_USE_AVDECC == 1)
#include "aem_manager.h"
#endif
#include "board.h"

struct net_config {
    uint8_t hw_addr[6];
    uint8_t ip_addr[4];
    uint8_t net_mask[4];
    uint8_t gw_addr[4];
};

struct tsn_app_config {
    unsigned int mode;
    unsigned int role;
    unsigned int num_io_devices;
    float motor_offset;
    unsigned int control_strategy;
    unsigned int use_st;
    unsigned int use_fp;
    unsigned int cmd_client;
    unsigned int period_ns;
};

struct avb_app_config {
    unsigned int mclock_role;
};

struct app_config {
    union {
        struct tsn_app_config tsn_app_config;
        struct avb_app_config avb_app_config;
    };
};

#if (CONFIG_GENAVB_USE_AVDECC == 1)
struct avb_avdecc_config {
    bool milan_mode;
    aem_entity_id_t aem_id;
};
#endif

struct system_config {
    struct app_config app;
    struct net_config net[BOARD_NUM_PORTS];
#if (CONFIG_GENAVB_USE_AVDECC == 1)
    struct avb_avdecc_config avdecc;
#endif
};

#if (CONFIG_GENAVB_USE_AVDECC == 1)
void system_config_set_avdecc(aem_entity_id_t id, bool milan_mode);
struct avb_avdecc_config *system_config_get_avdecc(void);
#endif
int system_config_set_net(unsigned int port_id, uint8_t *hw_addr);
struct net_config *system_config_get_net(unsigned int port_id);
struct app_config *system_config_get_app(void);
struct avb_app_config *system_config_get_avb_app(void);
uint32_t system_config_get_app_mode(void);

#endif /* _SYSTEM_CONFIG_H_ */
