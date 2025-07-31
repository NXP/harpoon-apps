/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_app.h"
#include "aem_manager.h"
#include "os/irq.h"
#include "rtos_apps/audio/audio_ctrl.h"
#include "rtos_apps/audio/audio_entry.h"
#include "rtos_apps/log.h"

#include "rtos_abstraction_layer.h"

#include "rpmsg.h"

#if defined(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE)
#include "avb_hardware.h"
#include "genavb.h"
#include "stats_task.h"

#include "genavb_sdk.h"
#include "system_config.h"

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

#endif /* #if defined(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE) */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DATA_TASK_PRIORITY   (RTOS_MAX_PRIORITY - 2)
#define CTRL_TASK_PRIORITY   (RTOS_MAX_PRIORITY - 8)
#define MAIN_TASK_PRIORITY   (RTOS_MAX_PRIORITY - 10)

#ifdef CONFIG_SMP
#define DATA_THREADS         2
#else
#define DATA_THREADS         1
#endif
#define STATS_PERIOD_MS      10000
#define DATA_TASK_STACK_SIZE (RTOS_MINIMAL_STACK_SIZE + 200)
#define CTRL_TASK_STACK_SIZE (RTOS_MINIMAL_STACK_SIZE + 800)
#define MAIN_TASK_STACK_SIZE (RTOS_MINIMAL_STACK_SIZE + 800)

const int audio_app_supported_period[] = {2, 4, 8, 16, 32};

static rtos_thread_t audio_thread;

/*******************************************************************************
 * Code
 ******************************************************************************/

#define EPT_ADDR	(30)

int audio_app_ctrl_send(void *ctrl_handle, void *data, uint32_t len)
{
	struct rpmsg_ept *ept = (struct rpmsg_ept *)ctrl_handle;

	return rpmsg_send(ept, data, len);
}

struct audio_hw_config {
	uint8_t address[6];
};

static void audio_set_hw_addr(struct audio_hw_config *cfg, uint8_t *hw_addr)
{
	uint8_t *addr = cfg->address;

	memcpy(addr, hw_addr, sizeof(cfg->address));

	log_info("%02x:%02x:%02x:%02x:%02x:%02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static void audio_app_setup(struct audio_cmd_run *run)
{
	unsigned int aem_id = AEM_ENTITY_TALKER_LISTENER_AUDIO_DEFAULT_ID;
	bool milan_mode = false;
	struct audio_hw_config cfg;

	audio_set_hw_addr(&cfg, run->addr);

	if (system_config_set_net(0, cfg.address)) {
		log_warn("system_config_set_net() failed\n");
	}

	if (run->id == 6 || run->id == 8) {
		milan_mode = true;
		/* Use Milan Entity only for AVB Pipelines with MCR Enabled */
		aem_id = AEM_ENTITY_LISTENER_TALKER_AUDIO_SINGLE_MILAN_ID;
	}

	system_config_set_avdecc(aem_id, milan_mode);
}

static int rpmsg_receive_audio_command(void *ctrl_handle, void *data, uint32_t *len)
{
	struct rpmsg_ept *ept = (struct rpmsg_ept *)ctrl_handle;

	return rpmsg_recv(ept, data, len);
}

int audio_app_ctrl_recv(void *ctrl_handle, void *data, uint32_t *len)
{
	struct audio_command *cmd;
	int rc;

	rc = rpmsg_receive_audio_command(ctrl_handle, data, len);
	if (rc < 0)
		return rc;

	cmd = (struct audio_command*)data;

	switch (cmd->u.cmd.type) {
	case AUDIO_CMD_TYPE_RUN:
		if (*len != sizeof(struct audio_cmd_run)) {
			break;
		}

		audio_app_setup(&cmd->u.audio_run);

		break;
	default:
		break;
	}

	return rc;
}

void *audio_app_ctrl_init(void)
{
	void *ctrl_handle = (void *)rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw");

	return ctrl_handle;
}

bool audio_app_check_params(uint32_t period, uint32_t rate)
{
	if (period == 2) {
		switch (rate) {
		case 176400:
		case 192000:
			log_warn("Unsupported rate(%u Hz)/period(%u) combination\n", rate, period);
			break;
		default:
			break;
		}
	}
	return true;
}

int audio_app_init(void)
{
	struct rtos_apps_audio_config config = {
		.thread_count = DATA_THREADS,
		.data_stack_size = DATA_TASK_STACK_SIZE,
		.data_priority = DATA_TASK_PRIORITY,
		.ctrl_stack_size = CTRL_TASK_STACK_SIZE,
		.ctrl_priority = CTRL_TASK_PRIORITY,
		.ctrl_handle = audio_app_ctrl_init(),
	};

	if (rtos_apps_audio_init(&config) < 0)
		goto err_init;

	return 0;

err_init:
	return -1;
}

static void main_task(void *data)
{
	log_info("Audio application started!\n");

	if (audio_app_init() < 0) {
		log_err("audio_app_init() failed\n");
		goto exit;
	}

	/* nothing else to do, exit */

exit:
#if (FSL_RTOS_FREE_RTOS)
	vTaskDelete(NULL);
#endif
}

void audio_app_main(void)
{
	(void)rtos_thread_create(&audio_thread, MAIN_TASK_PRIORITY, 0, MAIN_TASK_STACK_SIZE, "main task", main_task, NULL);
}

#if defined(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE)
struct genavb_handle *audio_app_avb_init(void)
{
	int rc;

	log_info("enter\n");

	avb_hardware_init();

	os_irq_register(BOARD_NET_PORT0_DRV_IRQ0, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ0_HND, NULL,
					OS_IRQ_PRIO_DEFAULT);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ1, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ1_HND, NULL,
					OS_IRQ_PRIO_DEFAULT);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ2, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ2_HND, NULL,
					OS_IRQ_PRIO_DEFAULT);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ3, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ3_HND, NULL,
					OS_IRQ_PRIO_DEFAULT);
#endif

	os_irq_register(BOARD_GENAVB_TIMER_0_IRQ, (void (*)(void(*)))BOARD_GENAVB_TIMER_0_IRQ_HANDLER, NULL,
					OS_IRQ_PRIO_DEFAULT);

	rc = gavb_stack_init();
	if (rc) {
		log_err("gavb_stack_init() failed\n");
		goto err;
	}

	if (STATS_TaskInit(NULL, NULL, STATS_PERIOD_MS) < 0)
		log_err("STATS_TaskInit() failed\n");

	return get_genavb_handle();

err:
	return NULL;
}

void audio_app_avb_exit(void)
{
	if (gavb_stack_exit()) {
		log_err("gavb_stack_exit() failed\n");
	}

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

	os_irq_unregister(BOARD_GENAVB_TIMER_0_IRQ);

	STATS_TaskExit();

	avb_hardware_exit();
}
#endif
