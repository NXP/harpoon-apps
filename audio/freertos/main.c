/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "log.h"
#include "os/assert.h"

#include "pin_mux.h"
#include "sai_clock_config.h"

#include "audio_entry.h"

#define main_task_PRIORITY   (configMAX_PRIORITIES - 3)
#define data_task_PRIORITY   (configMAX_PRIORITIES - 2)

static void hardware_setup(void)
{
	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	BOARD_InitPins();

	sai_clock_setup();
}

static void data_task(void *pvParameters)
{
	do {
		audio_process_data(pvParameters);
	} while (1);
}

void main_task(void *pvParameters)
{
	void *context;
	BaseType_t xResult;

	context = audio_control_init();
	os_assert(context, "control initialization failed!");

	xResult = xTaskCreate(data_task, "data_task",
                        configMINIMAL_STACK_SIZE + 100, context,
                        data_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "data task creation failed");

	/* forever loop */
	audio_control_loop(context);
}

int main(void)
{
	BaseType_t xResult;

	hardware_setup();

	log_info("Audio application started!\n");

	xResult = xTaskCreate(main_task, "main_task",
			configMINIMAL_STACK_SIZE + 200, NULL,
			main_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "main task creation failed");

	vTaskStartScheduler();

	for (;;)
		;

	return xResult;
}
