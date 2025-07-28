/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "clock_config.h"
#include "rtos_apps/log.h"
#include "rtos_abstraction_layer.h"

#include "pin_mux.h"

#include "audio_app.h"

static void hardware_setup(void)
{
	/* Init all clocks before any other init functions (especially uart init) */
	BOARD_InitClocks();

	BOARD_InitMemory();

	BOARD_InitDebugConsole();

	BOARD_RdcInit();

	BOARD_InitPins();
}

int main(void)
{
	hardware_setup();

	audio_app_main();

	vTaskStartScheduler();

	return pdPASS;
}
