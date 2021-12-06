/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "os.h"
#include "os/assert.h"
#include "os/stdio.h"

#include "sai_clock_config.h"
#include "sai_codec_config.h"
#include "sai_drv.h"
#include "audio.h"

#define main_task_PRIORITY	(configMAX_PRIORITIES - 3)
#define data_task_PRIORITY   (configMAX_PRIORITIES - 2)

static void hardware_setup(void)
{
	uint8_t sai_id = get_sai_id(DEMO_SAI);

	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	board_clock_setup(sai_id);
}

void (*data_task[])(void *) =
{
	[0] = play_dtmf_task,
	[1] = play_music_task,
	[2] = play_sine_task,
	[3] = rec_play_task,
	[4] = rec_play2_task,
};

void main_task(void *pvParameters)
{
	BaseType_t xResult;

	xResult = xTaskCreate(data_task[DEMO_MODE], "main_task",
                        configMINIMAL_STACK_SIZE + 100, NULL,
                        data_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "data task creation failed");

	do {
		vTaskDelay(pdMS_TO_TICKS(10000));

	} while(1);
}

int main(void)
{
	BaseType_t xResult;

	hardware_setup();

	os_printf("Audio application started!\r\n");

	xResult = xTaskCreate(main_task, "main_task",
			configMINIMAL_STACK_SIZE + 100, NULL,
			main_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "main task creation failed");

	vTaskStartScheduler();

	for (;;)
		;

	return xResult;
}
