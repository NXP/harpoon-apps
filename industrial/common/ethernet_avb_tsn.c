/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "hlog.h"
#include "hrpn_ctrl.h"
#include "industrial.h"

#include "alarm_task.h"
#include "avb_tsn/genavb.h"
#include "avb_tsn/stats_task.h"
#include "cyclic_task.h"
#include "hardware_enet_qos.h"
#include "ethernet.h"
#include "industrial.h"

#include "os/irq.h"
#include "os/stdlib.h"
#include "os/unistd.h"

#include "system_config.h"
#include "tsn_tasks_config.h"

#define STATS_PERIOD_MS 2000

extern void ENET_QOS_DriverIRQHandler(void);
extern void BOARD_GPT_0_IRQ_HANDLER(void);
extern void BOARD_GPT_1_IRQ_HANDLER(void);

#if BUILD_MOTOR == 1
static struct controller_ctx ctrl1;
static struct io_device_ctx io_device1;
static struct controller_ctx *ctrl_h = NULL;
static struct io_device_ctx *io_device_h = NULL;
#endif

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

static struct tsn_app_config *system_config_get_tsn_app(void)
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
	log_info("not implemented\n");
}

int ethernet_avb_tsn_run(void *priv, struct event *e)
{
	struct ethernet_ctx *ctx = priv;
	const struct tsn_app_config *config;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;

	avb_tsn_ctx = (struct ethernet_avb_tsn_ctx *)(ctx + 1);

	config = system_config_get_tsn_app();
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

	if (config->period_ns < APP_PERIOD_MIN) {
		log_err("invalid application period, minimum is %u ns\n", APP_PERIOD_MIN);
		goto exit;
	}

#if BUILD_MOTOR == 1
	if ((config->mode == MOTOR_LOCAL || config->mode == MOTOR_NETWORK) &&
		(config->period_ns != 100000) && (config->period_ns != 250000)) {
		log_err("invalid application period, only 100000 us and 250000 us are supported\n");
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
#else
	log_info("BUILD_MOTOR disabled, MOTOR_NETWORK and MOTOR_LOCAL modes cannot be used\n");
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

#if BUILD_MOTOR == 1
	if (config->mode == MOTOR_NETWORK || config->mode == MOTOR_LOCAL) {
		if (avb_tsn_ctx->c_task->type == CYCLIC_CONTROLLER) {
			if (controller_init(&ctrl1, avb_tsn_ctx->c_task, config->mode == MOTOR_LOCAL,
					(control_strategies_t)config->control_strategy, (bool)config->cmd_client) < 0) {
				log_err("Controller initialization failed\n");
				goto exit;
			}
			ctrl_h = &ctrl1;
		} else if (avb_tsn_ctx->c_task->type == CYCLIC_IO_DEVICE) {
			if (io_device_init(&io_device1, avb_tsn_ctx->c_task, 1, false) < 0) {
				log_err("io_device initialization failed\n");
				goto exit;
			}
			io_device_h = &io_device1;
			io_device_set_motor_offset(io_device_h, 0, config->motor_offset);
		} else {
			log_err("Unknown cyclic task type\n");
			goto exit;
		}
	} else
#endif /* BUILD_MOTOR */
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
	else if (avb_tsn_ctx->a_task->type == ALARM_IO_DEVICE) {
		alarm_task_io_init(avb_tsn_ctx->a_task);

		while (true) {
			os_msleep(10000);
			alarm_net_transmit(avb_tsn_ctx->a_task, 0, NULL, 0);
		}
	}

exit:
	return 0;
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

	gavb_pps_exit(&avb_tsn_ctx->pps);
	gavb_port_stats_exit(0);

	if (gavb_stack_exit()) {
		log_err("gavb_stack_exit() failed\n");
	}

	irq_unregister(BOARD_GPT_1_IRQ);
	irq_unregister(BOARD_GPT_0_IRQ);
	irq_unregister(BOARD_ENET0_DRV_IRQ);

	os_free(ctx);

	log_info("end\n");
}

void *ethernet_avb_tsn_init(void *parameters)
{
	struct industrial_config *cfg = parameters;
	struct ethernet_avb_tsn_ctx *avb_tsn_ctx;
	struct ethernet_ctx *ctx;

	ctx = os_malloc(sizeof(*ctx) + sizeof(*avb_tsn_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");

		goto exit;
	}

	avb_tsn_ctx = (struct ethernet_avb_tsn_ctx *)(ctx + 1);
	memset(ctx, 0, sizeof(*ctx) + sizeof(*avb_tsn_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	log_info("%s\n", __func__);

	enet_qos_hardware_init();

	irq_register(BOARD_ENET0_DRV_IRQ, (void (*)(void(*)))ENET_QOS_DriverIRQHandler, NULL);
	irq_register(BOARD_GPT_0_IRQ, (void (*)(void(*)))BOARD_GPT_0_IRQ_HANDLER, NULL);
	irq_register(BOARD_GPT_1_IRQ, (void (*)(void(*)))BOARD_GPT_1_IRQ_HANDLER, NULL);

	if (STATS_TaskInit(NULL, NULL, STATS_PERIOD_MS) < 0)
		log_err("STATS_TaskInit() failed\n");

exit:
	return ctx;
}
