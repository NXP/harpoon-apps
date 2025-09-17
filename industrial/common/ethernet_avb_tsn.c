/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "genavb/sr_class.h"
#include "rtos_apps/async.h"
#include "rtos_apps/log.h"
#include "hrpn_ctrl.h"
#include "industrial.h"
#include "rtos_abstraction_layer.h"

#include "log.h"
#include "genavb.h"
#include "stats_task.h"
#include "rtos_apps/types.h"
#include "ethernet.h"
#include "hardware_ethernet.h"
#include "industrial.h"
#include <stdio.h>

#include "os/irq.h"

#include "system_config.h"

#include "rtos_apps/tsn/tsn_tasks_config.h"
#include "rtos_apps/tsn/tsn_entry.h"

#include "genavb/frame_preemption.h"
#include "genavb/scheduled_traffic.h"
#include "genavb/ether.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CONFIG_USE_FP 0
#define CONFIG_USE_ST 1

#define NUM_IO_DEVICES_MIN 1
#define NUM_IO_DEVICES_MAX 2

#define APP_PERIOD_MAX 1000000000

#define STATS_PERIOD_MS 2000

extern void BOARD_NET_PORT0_DRV_IRQ0_HND(void);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
extern void BOARD_NET_PORT0_DRV_IRQ1_HND(void);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
extern void BOARD_NET_PORT0_DRV_IRQ2_HND(void);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
extern void BOARD_NET_PORT0_DRV_IRQ3_HND(void);
#endif

extern void BOARD_GENAVB_TIMER_0_IRQ_HANDLER(void);

#if SERIAL_MODE == 1
static struct serial_iodevice_ctx serial_iodev;
#endif

struct ethernet_avb_tsn_ctx {
	struct gavb_pps pps;
	struct rtos_apps_async *async;
	struct tsn_app_ctx *tsn_ctx;
};

extern struct system_config system_cfg;

#define ST_TX_TIME_MARGIN 1000 /* Additional margin to account for drift between MAC and gPTP clocks */
#define ST_TX_TIME_FACTOR 2 /* Factor applied to critical time interval, to avoid frames getting stuck */
#define ST_LIST_LEN       2

/*
 * Returns the complete transmit time including MAC framing and physical
 * layer overhead (802.3).
 * \return transmit time in nanoseconds
 * \param frame_size frame size without any framing
 * \param speed_mbps link speed in Mbps
 */
static unsigned int frame_tx_time_ns(unsigned int frame_size, int speed_mbps)
{
    unsigned int eth_size;

    eth_size = sizeof(struct eth_hdr) + frame_size + ETHER_FCS;

    if (eth_size < ETHER_MIN_FRAME_SIZE)
        eth_size = ETHER_MIN_FRAME_SIZE;

    eth_size += ETHER_IFG + ETHER_PREAMBLE;

    return (((1000 / speed_mbps) * eth_size * 8) + ST_TX_TIME_MARGIN);
}

#if CONFIG_USE_ST
#define SCHED_TRAFFIC_OFFSET 40000
static void tsn_net_st_config_enable(struct tsn_task_params *params, bool use_fp, bool use_st)
{
    struct genavb_st_config config;
    struct genavb_st_gate_control_entry gate_list[ST_LIST_LEN];
    struct net_address *addr = &params->tx_params[0].addr;
    unsigned int cycle_time = params->task_period_ns;
    uint8_t iso_traffic_prio = addr->priority;
    const uint8_t *map;
    uint8_t tclass;
    unsigned int iso_tx_time = frame_tx_time_ns(params->tx_buf_size, 1000) * ST_TX_TIME_FACTOR;
    int i;

    map = priority_to_traffic_class_map(CFG_TRAFFIC_CLASS_MAX, CFG_SR_CLASS_MAX);
    if (!map) {
        log_err("priority_to_traffic_class_map() error\n");
        return;
    } else {
        tclass = map[iso_traffic_prio];
    }

    gate_list[0].gate_states = 1 << tclass;

    if (use_fp) {
        gate_list[0].operation = GENAVB_ST_SET_AND_HOLD_MAC;

        /*
         * Keep preemptable queues always open.
         * Match configuration done in tsn_net_fp_config_enable().
         */
        for (i = 0; i < tclass; i++)
            gate_list[0].gate_states |= 1 << i;
    } else {
        gate_list[0].operation = GENAVB_ST_SET_GATE_STATES;
    }

    gate_list[0].time_interval = iso_tx_time;

    if (use_fp)
        gate_list[1].operation = GENAVB_ST_SET_AND_RELEASE_MAC;
    else
        gate_list[1].operation = GENAVB_ST_SET_GATE_STATES;

    gate_list[1].gate_states = ~(1 << tclass);
    gate_list[1].time_interval = cycle_time - iso_tx_time;

    /* Scheduled traffic will start when (base_time + N * cycle_time) > now */
    config.enable = 1;
    config.base_time = params->task_period_offset_ns + SCHED_TRAFFIC_OFFSET;
    config.cycle_time_p = cycle_time;
    config.cycle_time_q = NSECS_PER_SEC;
    config.cycle_time_ext = 0;
    config.list_length = ST_LIST_LEN;
    config.control_list = gate_list;

    if (genavb_st_set_admin_config(addr->port, params->clk_id, &config) < 0)
        log_err("genavb_st_set_admin_config() error\n");
    else
        log_info("scheduled traffic config enabled\n");
}

static void tsn_net_st_config_disable(struct tsn_task_params *params)
{
    struct genavb_st_config config;
    struct net_address *addr = &params->tx_params[0].addr;

    config.enable = 0;

    if (genavb_st_set_admin_config(addr->port, params->clk_id, &config) < 0)
        log_err("genavb_st_set_admin_config() error\n");
    else
        log_info("scheduled traffic config disabled\n");
}
#endif

#if CONFIG_USE_FP
static void tsn_net_fp_config_enable(struct tsn_task_params *params)
{
    struct genavb_fp_config config;
    struct net_address *addr = &params->tx_params[0].addr;
    const uint8_t *map;
    uint8_t tclass;
    int i;

    map = priority_to_traffic_class_map(CFG_TRAFFIC_CLASS_MAX, CFG_SR_CLASS_MAX);
    if (!map) {
        log_err("priority_to_traffic_class_map() error\n");
        return;
    } else {
        tclass = map[addr->priority];
    }

    for (i = 0; i < QOS_PRIORITY_MAX; i++) {
        if (map[i] >= tclass)
            config.u.cfg_802_1Q.admin_status[i] = GENAVB_FP_ADMIN_STATUS_EXPRESS;
        else
            config.u.cfg_802_1Q.admin_status[i] = GENAVB_FP_ADMIN_STATUS_PREEMPTABLE;
    }

    if (genavb_fp_set(addr->port, GENAVB_FP_CONFIG_802_1Q, &config) < 0) {
        log_err("genavb_fp_set(802.1Q) error\n");
        return;
    }

    config.u.cfg_802_3.enable_tx = 1;
    config.u.cfg_802_3.verify_disable_tx = 0;
    config.u.cfg_802_3.verify_time = 100;
    config.u.cfg_802_3.add_frag_size = 0;

    if (genavb_fp_set(addr->port, GENAVB_FP_CONFIG_802_3, &config) < 0) {
        log_err("genavb_fp_set(802.3) error\n");
        return;
    }
}
#endif

static struct rtos_apps_tsn_config *system_config_get_tsn_app(struct ethernet_ctx *ctx)
{
	struct rtos_apps_tsn_config *config = &system_cfg.app.tsn_app_config;

	config->mode = ctx->app_mode;
	config->role = ctx->role;
	config->period_ns = ctx->period;
	config->num_io_devices = ctx->num_io_devices;
	config->control_strategy = ctx->control_strategy;

	return config;
}

void ethernet_avb_tsn_stats(void *priv)
{
	struct ethernet_ctx *ctx = priv;

	(void)ctx;
	log_debug("not implemented\n");
}

int ethernet_avb_tsn_run(void *priv, struct event *e)
{
	struct ethernet_ctx *ctx = priv;
	struct cyclic_task_config *c_cfg;
	struct rtos_apps_tsn_config *config;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;

#if BUILD_SERIAL == 1
	struct rtos_apps_tsn_serial_iodevice_config serial_cfg = {
		.baseaddr = BOARD_IODEV_UART_BASEADDR,
		.baudrate = BOARD_IODEV_UART_BAUDRATE,
		.clk_freq = BOARD_IODEV_UART_CLK_FREQ,
		.irq_mask = BOARD_IODEV_UART_INTERRUPT_MASK,
	};
#endif

	avb_tsn_ctx = (struct ethernet_avb_tsn_ctx *)(ctx + 1);

	config = system_config_get_tsn_app(ctx);
	if (!config) {
		log_err("system_config_get_tsn_app() failed\n");
		goto exit;
	}

	if (gavb_stack_init()) {
		log_err("gavb_stack_init() failed\n");
		goto exit;
	}

	if (gavb_port_stats_init(0)) {
		log_err("gavb_port_stats_init() failed\n");
		goto err_stats;
	}

#if BUILD_SERIAL == 1
	config->serial_cfg = &serial_cfg;
#endif

	config->log_update_time = app_log_update_time;
	config->async = avb_tsn_ctx->async;

	c_cfg = tsn_conf_get_cyclic_task(config->role);
	if (!c_cfg) {
		log_err("tsn_conf_get_cyclic_task() failed\n");
		goto err_init;
	}

	if (gavb_pps_init(&avb_tsn_ctx->pps, c_cfg->params.clk_id) < 0)
		log_err("gavb_pps_init() failed: pps timer could not be started\n");

#if CONFIG_USE_FP
	tsn_net_fp_config_enable(&c_cfg->params);
#endif

#if CONFIG_USE_ST
	tsn_net_st_config_enable(&c_cfg->params, CONFIG_USE_FP, CONFIG_USE_ST);
#endif

	if (rtos_apps_tsn_init(config, &avb_tsn_ctx->tsn_ctx) < 0)
		goto err_init;

	return 0;

err_init:
	gavb_port_stats_exit(0);

err_stats:
	if (gavb_stack_exit() < 0)
		log_err("gavb_stack_exit() failed\n");

exit:
	return -1;
}

void ethernet_avb_tsn_exit(void *priv)
{
	struct ethernet_ctx *ctx = priv;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;
	struct rtos_apps_tsn_config *config;
	struct cyclic_task_config *c_cfg;

	avb_tsn_ctx = (struct ethernet_avb_tsn_ctx *)(ctx + 1);

	rtos_apps_tsn_exit(avb_tsn_ctx->tsn_ctx);

	gavb_pps_exit(&avb_tsn_ctx->pps);
	gavb_port_stats_exit(0);

	if (gavb_stack_exit()) {
		log_err("gavb_stack_exit() failed\n");
	}

	os_irq_unregister(BOARD_GENAVB_TIMER_0_IRQ);
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ0);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ1);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ2);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ3);
#endif

	config = system_config_get_tsn_app(ctx);
	if (!config) {
		log_err("system_config_get_tsn_app() failed\n");
		return;
	}

	c_cfg = tsn_conf_get_cyclic_task(config->role);
	if (!c_cfg) {
		log_err("tsn_conf_get_cyclic_task() failed\n");
		return;
	}

#if CONFIG_USE_ST
	tsn_net_st_config_disable(&c_cfg->params);
#endif

	rtos_free(ctx);

	log_info("end\n");
}

void *ethernet_avb_tsn_init(void *parameters)
{
	struct industrial_config *cfg = parameters;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;
	struct ethernet_ctx *ctx = NULL;

	/* validate user defined task period: check range and if it is an integer that can divide 1 second */
	if (cfg->period < APP_PERIOD_MIN || cfg->period > APP_PERIOD_MAX || ((NSECS_PER_SEC / cfg->period) * cfg->period != NSECS_PER_SEC)) {
		log_err("Period %d frames is not supported\n", cfg->period);
		goto exit;
	}

	if (cfg->num_io_devices < NUM_IO_DEVICES_MIN || cfg->num_io_devices > NUM_IO_DEVICES_MAX) {
		log_err("Unsupported number of iodevices (%u)\n", cfg->num_io_devices);
		goto exit;
	}

	if (cfg->control_strategy > CTRL_STRAT_IDENTIFICATION) {
		log_err("Unsupported control strategy (%u)\n", cfg->control_strategy);
		goto exit;
	}
	
	if (cfg->app_mode == NETWORK_ONLY) {
		/* sanity check */
		if (cfg->role >= MAX_TASKS_ID) {
			log_err("Invalid role: %d\n", cfg->role);
			goto exit;
		}
	}
#if BUILD_MOTOR_CONTROLLER == 1
	else if (cfg->app_mode == MOTOR_NETWORK) {
		if ((cfg->role == IO_DEVICE_0 || cfg->role == IO_DEVICE_1 || cfg->role >= MAX_TASKS_ID)) {
			log_err("Unsupported role (%u), motor control is supported in CONTROLLER role only\n", cfg->role);
			goto exit; 
		}
	}
#endif
	else {
		log_err("Unsupported TSN app mode (%u)\n", cfg->app_mode);
		goto exit;
	}

	ctx = rtos_malloc(sizeof(*ctx) + sizeof(*avb_tsn_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");
		goto exit;
	}

	avb_tsn_ctx = (struct ethernet_avb_tsn_ctx *)(ctx + 1);
	memset(ctx, 0, sizeof(*ctx) + sizeof(*avb_tsn_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;
	ctx->period = cfg->period;
	ctx->role = cfg->role;
	ctx->num_io_devices = cfg->num_io_devices;
	ctx->control_strategy = cfg->control_strategy;
	ctx->app_mode = cfg->app_mode;

	memcpy(ctx->mac_addr, cfg->address, sizeof(ctx->mac_addr));

	log_info("%s\n", __func__);

	if (system_config_set_net(0, ctx->mac_addr)) {
		log_warn("system_config_set_net() failed\n");
	}

	hardware_ethernet_init();

	os_irq_register(BOARD_NET_PORT0_DRV_IRQ0, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ0_HND, NULL, OS_IRQ_PRIO_DEFAULT);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ1, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ1_HND, NULL, OS_IRQ_PRIO_DEFAULT);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ2, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ2_HND, NULL, OS_IRQ_PRIO_DEFAULT);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ3, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ3_HND, NULL, OS_IRQ_PRIO_DEFAULT);
#endif

	os_irq_register(BOARD_GENAVB_TIMER_0_IRQ, (void (*)(void(*)))BOARD_GENAVB_TIMER_0_IRQ_HANDLER, NULL, OS_IRQ_PRIO_DEFAULT);

	if (STATS_TaskInit(NULL, NULL, STATS_PERIOD_MS, &avb_tsn_ctx->async) < 0)
		log_err("STATS_TaskInit() failed\n");

exit:
	return ctx;
}
