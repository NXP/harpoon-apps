/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "app_board.h"
#include "hlog.h"
#include "os/assert.h"

#include "pin_mux.h"
#include "sai_clock_config.h"

#include "audio_entry.h"

#define STACK_SIZE 4096

K_THREAD_STACK_DEFINE(data_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(ctrl_stack, STACK_SIZE);

static void hardware_setup(void)
{
	BOARD_InitPins();

	sai_clock_setup();
}

static void data_task(void *context, void *p2, void *p3)
{
	do {
		audio_process_data(context);
	} while (1);
}

static void ctrl_task(void *context, void *p2, void *p3)
{
	audio_control_loop(context);
}

void main(void)
{
	struct k_thread data_thread, ctrl_thread;
	void *context;

	log_info("Audio application started!\n");

	hardware_setup();

	context = audio_control_init();
	os_assert(context, "control initialization failed!");

	k_thread_create(&data_thread, data_stack, STACK_SIZE,
		data_task, context, NULL, NULL,
		K_HIGHEST_THREAD_PRIO, 0, K_NO_WAIT);

	k_thread_create(&ctrl_thread, ctrl_stack, STACK_SIZE,
		ctrl_task, context, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
}
