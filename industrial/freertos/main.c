/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "hlog.h"
#include "os/assert.h"

#include "clock_config.h"
#include "pin_mux.h"

#include "industrial_entry.h"
#include "industrial_os.h"

#define main_task_PRIORITY	(configMAX_PRIORITIES - 8)

const struct industrial_use_case use_cases[] =
{
	[INDUSTRIAL_USE_CASE_CAN] = {
		.thread = {
			.name = "can_event",
			.nb_threads = 1,
			.priority = 1,
		},
		.ops = {
			[0] = {
				.init = can_init_loopback,
				.exit = can_exit,
				.run = can_run,
				.stats = can_stats,
			},
			[1] = {
				.init = can_init_interrupt,
				.exit = can_exit,
				.run = can_run,
				.stats = can_stats,
			},
			[2] = {
				.init = can_init_pingpong,
				.exit = can_exit,
				.run = can_run,
				.stats = can_stats,
			},
		},
	},
	[INDUSTRIAL_USE_CASE_ETHERNET] = {
		.thread = {
			.name = "ethernet_event",
			.nb_threads = 1,
			.priority = 1,
		},
		.ops = {
			[0] = {
				.init = ethernet_avb_tsn_init,
				.exit = ethernet_avb_tsn_exit,
				.run = ethernet_avb_tsn_run,
				.stats = ethernet_avb_tsn_stats,
			},
		},
	},
};

static void hardware_setup(void)
{
	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	BOARD_InitPins();

	board_clock_setup();
}

static void data_task(void *context)
{
	struct data_ctx *data = context;

	do {
		data->process_data(context);
	} while (1);
}

static int create_task(unsigned int use_case_id, void *data)
{
	BaseType_t xResult;
	unsigned int i = use_case_id;
	const struct thread_cfg *t = &use_cases[i].thread;

	log_debug("ctx %p: thread config: %d %d %s\n", data,
		t->nb_threads, t->priority, t->name);

	xResult = xTaskCreate(data_task, t->name,
		configMINIMAL_STACK_SIZE + 800, data,
		t->priority, NULL);

	return !(xResult == pdPASS);
}

void main_task(void *pvParameters)
{
	int nb_use_cases = ARRAY_SIZE(use_cases);
	void *context = NULL;
	int i, ret;

	context = industrial_control_init(nb_use_cases);

	for (i = 0; i < nb_use_cases; i++) {
		void *data = industrial_get_data_ctx(context, i);

		ret = create_task(i, data);
		os_assert(!ret, "task creation failed for ctx %d", i);
	}

	do {
		industrial_control_loop(context);
	} while(1);
}

int main(void)
{
	BaseType_t xResult;

	hardware_setup();

	log_info("Industrial application started!\n");

	xResult = xTaskCreate(main_task, "main_task",
			configMINIMAL_STACK_SIZE + 100, NULL,
			main_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "main task creation failed");

	vTaskStartScheduler();

	for (;;)
		;

	return xResult;
}
