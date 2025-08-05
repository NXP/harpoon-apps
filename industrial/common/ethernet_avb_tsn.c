/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "rtos_apps/async.h"
#include "rtos_apps/log.h"
#include "hrpn_ctrl.h"
#include "industrial.h"
#include "rtos_abstraction_layer.h"

#include "alarm_task.h"
#include "avb_tsn/genavb.h"
#include "avb_tsn/stats_task.h"
#include "avb_tsn/types.h"
#include "cyclic_task.h"
#include "ethernet.h"
#include "hardware_ethernet.h"
#include "industrial.h"

#include "os/irq.h"

#include "system_config.h"
#include "tsn_tasks_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
struct rtos_apps_async *async;

#if BUILD_MOTOR_CONTROLLER == 1
#include "controller.h"
#endif

#if BUILD_MOTOR_IO_DEVICE == 1
#include "io_device.h"
#include "local_network.h"
#endif

#define STATS_PERIOD_MS 2000

#if BUILD_MOTOR_CONTROLLER == 1
static struct controller_ctx ctrl1;
static struct controller_ctx *ctrl_h = NULL;
#endif

#if BUILD_MOTOR_IO_DEVICE == 1
static struct io_device_ctx io_device1;
static struct io_device_ctx *io_device_h = NULL;
#endif

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
	struct cyclic_task *c_task;
	struct cyclic_task *opt_io_device_task;
	struct alarm_task *a_task;
};

static char *app_mode_names[] = {"MOTOR_NETWORK", "MOTOR_LOCAL", "NETWORK_ONLY", "SERIAL"};

extern struct system_config system_cfg;

static struct tsn_app_config *system_config_get_tsn_app(struct ethernet_ctx *ctx)
{
	struct tsn_app_config *config = &system_cfg.app.tsn_app_config;

#if (ENABLE_STORAGE == 1)
	if (storage_cd("/tsn_app") == 0) {
		storage_read_uint("mode", &config->mode);
		storage_read_uint("role", &config->role);
		storage_read_uint("num_io_devices", &config->num_io_devices);
		storage_read_float("motor_offset", &config->motor_offset);
		storage_read_uint("control_strategy", &config->control_strategy);
		storage_read_uint("use_st", &config->use_st);
		storage_read_uint("use_fp", &config->use_fp);
		storage_read_uint("cmd_client", &config->cmd_client);

		if (config->mode == SERIAL)
			config->period_ns = APP_PERIOD_SERIAL_DEFAULT;

		storage_read_uint("period_ns", &config->period_ns);

		storage_cd("/");
	}
#else
	config->mode = ctx->app_mode;
	config->role = ctx->role;
	config->period_ns = ctx->period;
	config->num_io_devices = ctx->num_io_devices;
	config->control_strategy = ctx->control_strategy;

#endif /* (ENABLE_STORAGE == 1) */

	return config;
}

static void null_loop(void *data, int timer_status)
{
	struct cyclic_task *c_task = data;

	cyclic_net_transmit(c_task, 0, NULL, 0);
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
	const struct tsn_app_config *config;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;

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
		goto exit;
	}

	avb_tsn_ctx->a_task = tsn_conf_get_alarm_task(config->role);
	if (!avb_tsn_ctx->a_task) {
		log_err("tsn_conf_get_alarm_task() failed\n");
		goto exit;
	}

	log_info("tsn_app config\n");
	log_info("mode             : %s\n", app_mode_names[config->mode]);
	log_info("role             : %u\n", config->role);
	log_info("num_io_devices   : %u\n", config->num_io_devices);
	log_info("motor_offset     : %f\n", config->motor_offset);
	log_info("control_strategy : %u\n", config->control_strategy);
	log_info("app period       : %u\n", config->period_ns);

#if ((BUILD_MOTOR_CONTROLLER == 0) && (BUILD_MOTOR_IO_DEVICE == 0))
	log_info("BUILD_MOTOR disabled, MOTOR_NETWORK and MOTOR_LOCAL modes cannot be used\n");
#endif

#if ((BUILD_MOTOR_CONTROLLER == 1) || (BUILD_MOTOR_IO_DEVICE == 1))
	if (config->mode == MOTOR_NETWORK) {
		if ((config->period_ns != 100000) && (config->period_ns != 250000)) {
			log_err("invalid application period, only 100000 ns and 250000 ns are supported\n");
			goto exit;
		}
	}
#endif

#if ((BUILD_MOTOR_CONTROLLER == 1) && (BUILD_MOTOR_IO_DEVICE == 1))
	if ((config->mode == MOTOR_LOCAL) &&
		(config->period_ns != 100000) && (config->period_ns != 250000)) {
		log_err("invalid application period, only 100000 ns and 250000 ns are supported\n");
		goto exit;
	}

	if (init_gpio_handling_task() < 0) {
		log_err("init_gpio_handling_task() failed\n");
		goto exit;
	}

	if (config->mode == MOTOR_LOCAL) {
		avb_tsn_ctx->c_task = tsn_conf_get_cyclic_task(0);
		if (!avb_tsn_ctx->c_task) {
			log_err("tsn_conf_get_cyclic_task() failed\n");
			goto exit;
		}

		cyclic_task_set_period(avb_tsn_ctx->c_task, config->period_ns);

		avb_tsn_ctx->c_task->num_peers = 0;
		avb_tsn_ctx->c_task->params.clk_id = GENAVB_CLOCK_MONOTONIC;

		avb_tsn_ctx->opt_io_device_task = tsn_conf_get_cyclic_task(1);
		if (!avb_tsn_ctx->opt_io_device_task) {
			log_err("tsn_conf_get_cyclic_task() failed\n");
			goto exit;
		}
		avb_tsn_ctx->opt_io_device_task->num_peers = 0;
		avb_tsn_ctx->opt_io_device_task->params.clk_id = GENAVB_CLOCK_MONOTONIC;

		cyclic_task_set_period(avb_tsn_ctx->opt_io_device_task, config->period_ns);

		if (io_device_init(&io_device1, avb_tsn_ctx->opt_io_device_task, 1, true) < 0) {
			log_err("Local io_device initialization failed\n");
			goto exit;
		}

		local_bind_controller_io_device(&ctrl1, &io_device1);

	} else
#endif /* BUILD_MOTOR */
	{
		avb_tsn_ctx->c_task = tsn_conf_get_cyclic_task(config->role);
		if (!avb_tsn_ctx->c_task) {
			log_err("tsn_conf_get_cyclic_task() failed\n");
			goto exit;
		}

		cyclic_task_set_period(avb_tsn_ctx->c_task, config->period_ns);
	}

	if (gavb_pps_init(&avb_tsn_ctx->pps, avb_tsn_ctx->c_task->params.clk_id) < 0)
		log_err("gavb_pps_init() error,pps timer could not be started\n");

	if (avb_tsn_ctx->c_task->type == CYCLIC_CONTROLLER) {
		avb_tsn_ctx->c_task->num_peers = config->num_io_devices;
		avb_tsn_ctx->a_task->num_peers = config->num_io_devices;
	}

	avb_tsn_ctx->c_task->params.use_st = config->use_st;
	avb_tsn_ctx->c_task->params.use_fp = config->use_fp;

#if (BUILD_MOTOR_CONTROLLER == 1) || (BUILD_MOTOR_IO_DEVICE == 1)
	if (config->mode == MOTOR_NETWORK || config->mode == MOTOR_LOCAL) {
#if BUILD_MOTOR_CONTROLLER == 1
		if (avb_tsn_ctx->c_task->type == CYCLIC_CONTROLLER) {
			if (controller_init(&ctrl1, avb_tsn_ctx->c_task, config->mode == MOTOR_LOCAL,
					(control_strategies_t)config->control_strategy, (bool)config->cmd_client) < 0) {
				log_err("Controller initialization failed\n");
				goto exit;
			}
			ctrl_h = &ctrl1;
		}
#endif
#if BUILD_MOTOR_IO_DEVICE == 1
		if (avb_tsn_ctx->c_task->type == CYCLIC_IO_DEVICE) {
			if (io_device_init(&io_device1, avb_tsn_ctx->c_task, 1, false) < 0) {
				log_err("io_device initialization failed\n");
				goto exit;
			}
			io_device_h = &io_device1;
			io_device_set_motor_offset(io_device_h, 0, config->motor_offset);
		} 
#endif 
		if (avb_tsn_ctx->c_task->type != CYCLIC_CONTROLLER && avb_tsn_ctx->c_task->type != CYCLIC_IO_DEVICE) {
			log_err("Unknown cyclic task type\n");
			goto exit;
		}
	} else
#endif /* (BUILD_MOTOR_CONTROLLER == 1) || (BUILD_MOTOR_IO_DEVICE == 1) */
	{
#if SERIAL_MODE == 1
		if (config->mode == SERIAL) {
			avb_tsn_ctx->c_task->params.task_period_ns = APP_PERIOD_SERIAL_DEFAULT;
			avb_tsn_ctx->c_task->params.task_period_offset_ns = NET_DELAY_OFFSET_SERIAL_DEFAULT;
			avb_tsn_ctx->c_task->params.transfer_time_ns = NET_DELAY_OFFSET_SERIAL_DEFAULT;

			if (serial_iodevice_init(&serial_iodev, avb_tsn_ctx->c_task) < 0) {
				log_err("serial_iodevice_init() failed\n");
				goto exit;
			}
		} else
#endif
			cyclic_task_init(avb_tsn_ctx->c_task, NULL, null_loop, avb_tsn_ctx->c_task);
	}

	cyclic_task_start(avb_tsn_ctx->c_task);

	if (avb_tsn_ctx->opt_io_device_task)
		cyclic_task_start(avb_tsn_ctx->opt_io_device_task);

	if (avb_tsn_ctx->a_task->type == ALARM_MONITOR)
		alarm_task_monitor_init(avb_tsn_ctx->a_task, NULL, NULL);
	else if (avb_tsn_ctx->a_task->type == ALARM_IO_DEVICE)
		alarm_task_io_init(avb_tsn_ctx->a_task);

	return 0;

exit:
	return -1;
}

void ethernet_avb_tsn_exit(void *priv)
{
	struct ethernet_ctx *ctx = priv;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;

	avb_tsn_ctx = (struct ethernet_avb_tsn_ctx *)(ctx + 1);

	if (avb_tsn_ctx->a_task) {
		if (avb_tsn_ctx->a_task->type == ALARM_MONITOR)
			alarm_task_monitor_exit(avb_tsn_ctx->a_task);
		else if (avb_tsn_ctx->a_task->type == ALARM_IO_DEVICE)
			alarm_task_io_exit(avb_tsn_ctx->a_task);
	}

	if (avb_tsn_ctx->opt_io_device_task)
		cyclic_task_stop(avb_tsn_ctx->opt_io_device_task);

	if (avb_tsn_ctx->c_task) {
		cyclic_task_stop(avb_tsn_ctx->c_task);
		cyclic_task_exit(avb_tsn_ctx->c_task);
	}

#if BUILD_MOTOR_CONTROLLER == 1
	if (ctx->app_mode == MOTOR_LOCAL || ctx->app_mode == MOTOR_NETWORK) {
		if (controller_exit(&ctrl1))
			log_err("controller_exit() failed\n");
	}
#endif
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

	if (cfg->control_strategy > IDENTIFY) {
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

	if (STATS_TaskInit(NULL, NULL, STATS_PERIOD_MS, &async) < 0)
		log_err("STATS_TaskInit() failed\n");

exit:
	return ctx;
}
