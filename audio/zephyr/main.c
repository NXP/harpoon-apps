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
#include "clock_config.h"

#include "audio.h"
#include "audio_entry.h"

#define STACK_SIZE 4096

#ifdef CONFIG_SMP
#define DATA_THREADS MAX_AUDIO_DATA_THREADS
#else
#define DATA_THREADS 1
#endif

K_THREAD_STACK_ARRAY_DEFINE(data_stack, DATA_THREADS, STACK_SIZE);
K_THREAD_STACK_DEFINE(ctrl_stack, STACK_SIZE);

static void hardware_setup(void)
{
	BOARD_InitPins();

	BOARD_InitClocks();
}

static void data_task(void *context, void *thread_id, void *p3)
{
	int id = *(int *)thread_id;

	do {
		audio_process_data(context, id);
	} while (1);
}

static void ctrl_task(void *context, void *p2, void *p3)
{
	audio_control_loop(context);
}

void main(void)
{
	struct k_thread data_thread[DATA_THREADS];
	struct k_thread ctrl_thread;
	void *context;
	int i;

	log_info("Audio application started!\n");

	hardware_setup();

	context = audio_control_init(DATA_THREADS);
	os_assert(context, "control initialization failed!");

	for (i = 0; i < DATA_THREADS; i++) {
		k_thread_create(&data_thread[i], data_stack[i], STACK_SIZE,
				data_task, context, &i, NULL,
				K_HIGHEST_THREAD_PRIO, 0, K_NO_WAIT);
	}

	k_thread_create(&ctrl_thread, ctrl_stack, STACK_SIZE,
		ctrl_task, context, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
}
