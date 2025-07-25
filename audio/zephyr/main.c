/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "hlog.h"
#include "rtos_abstraction_layer.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "app_mmu.h"

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
	BOARD_InitMemory();

	BOARD_InitPins();

	BOARD_InitClocks();
}

static void data_task(void *context, void *thread_id, void *p3)
{
	uint8_t id = (uint8_t)(uintptr_t)thread_id;

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
	k_tid_t thread_ret;
	int ret;
	void *context;
	int i;

	log_info("Audio application started!\n");

	hardware_setup();

	context = audio_control_init(DATA_THREADS);
	rtos_assert(context, "control initialization failed!");

	for (i = 0; i < DATA_THREADS; i++) {
		thread_ret = k_thread_create(&data_thread[i], data_stack[i], STACK_SIZE,
				data_task, context, (void *)(uintptr_t)i, NULL,
				K_HIGHEST_THREAD_PRIO, 0, K_FOREVER);
		rtos_assert(thread_ret != NULL, "k_thread_create() failed");
		ret = k_thread_cpu_pin(&data_thread[i], i);
		rtos_assert(ret == 0, "k_thread_cpu_pin() failed");
	}

	thread_ret = k_thread_create(&ctrl_thread, ctrl_stack, STACK_SIZE,
		ctrl_task, context, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_FOREVER);
	rtos_assert(thread_ret != NULL, "k_thread_create() failed");
	ret = k_thread_cpu_pin(&ctrl_thread, 0);
	rtos_assert(ret == 0, "k_thread_cpu_pin() failed");

	k_thread_start(&ctrl_thread);
	for (i = 0; i < DATA_THREADS; i++)
		k_thread_start(&data_thread[i]);
}
