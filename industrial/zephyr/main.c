/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "hlog.h"
#include "os/assert.h"
#include "os/stdlib.h"
#include "os/counter.h"

#include "clock_config.h"
#include "pin_mux.h"

#include "industrial_entry.h"
#include "industrial_os.h"

#define STACK_SIZE 4096

K_THREAD_STACK_DEFINE(data_stack1, STACK_SIZE);
K_THREAD_STACK_DEFINE(data_stack2, STACK_SIZE);
K_THREAD_STACK_DEFINE(ctrl_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(gpt_stack, STACK_SIZE);

const os_counter_t *os_counter = (os_counter_t *)DEVICE_DT_GET(DT_ALIAS(counter0));

struct z_thread_stack_element *data_stacks[] =
{
	[INDUSTRIAL_USE_CASE_CAN] = data_stack1,
	[INDUSTRIAL_USE_CASE_ETHERNET] = data_stack2,
};


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
				.init = can_init,
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
				.init = NULL,
				.exit = NULL,
				.run = NULL,
				.stats = NULL,
			},
			[1] = {
				.init = ethernet_sdk_enet_init,
				.exit = ethernet_sdk_enet_exit,
				.run = ethernet_sdk_enet_run,
				.stats = ethernet_sdk_enet_stats,
			},
			[2] = {
				.init = ethernet_sdk_enet_loopback_init,
				.exit = ethernet_sdk_enet_loopback_exit,
				.run = ethernet_sdk_enet_loopback_run,
				.stats = ethernet_sdk_enet_loopback_stats,
			},
		},
	},
};

static void hardware_setup(void)
{
	BOARD_InitPins();

	board_clock_setup();
}

static void data_task(void *context, void *p2, void *p3)
{
	struct data_ctx *data = context;

	do {
		data->process_data(context);
	} while (1);
}

static void ctrl_task(void *context, void *p2, void *p3)
{
	do {
		industrial_control_loop(context);
	} while(1);
}

void main_task(void)
{
	struct k_thread ctrl_thread;
	struct k_thread *data_thread;
	int nb_use_cases = ARRAY_SIZE(use_cases);
	void *context = NULL;
	int i;

	log_info("Industrial application started!\n");

	context = industrial_control_init(nb_use_cases);

	for (i = 0; i < nb_use_cases; i++) {
		void *data = industrial_get_data_ctx(context, i);
		const struct thread_cfg *t = &use_cases[i].thread;

		data_thread = os_malloc(sizeof(struct k_thread));
		os_assert((data_thread != NULL), "thread handle memory allocation error");

		log_debug("ctx %p: thread config: %d %d %s\n", data,
				t->nb_threads, t->priority, t->name);

		k_thread_create(data_thread, data_stacks[i], STACK_SIZE,
				data_task, data, NULL, NULL,
				K_HIGHEST_THREAD_PRIO, 0, K_NO_WAIT);
	}

	k_thread_create(&ctrl_thread, ctrl_stack, STACK_SIZE,
		ctrl_task, context, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
}

void main(void)
{
	hardware_setup();

	main_task();
}
