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
#include "test_sai.h"

#define test_task_PRIORITY	(configMAX_PRIORITIES - 1)

static void hardware_setup(void)
{
	uint8_t sai_id = get_sai_id(DEMO_SAI);

	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	board_clock_setup(sai_id);

	codec_setup();

	sai_setup(sai_id);
}

int main(void)
{
	BaseType_t xResult;

	hardware_setup();

	os_printf("SAI demo started!\n\r");

	xResult = xTaskCreate(sai_test_task, "sai_test_task",
			configMINIMAL_STACK_SIZE + 100, NULL,
			test_task_PRIORITY, NULL);
	os_assert(xResult == pdPASS, "Created sai test task failed");

	vTaskStartScheduler();

	for (;;)
		;

	return xResult;
}
