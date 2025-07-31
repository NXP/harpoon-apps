/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_mmu.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "audio_app.h"

static void hardware_setup(void)
{
	BOARD_InitMemory();

	BOARD_InitPins();

	BOARD_InitClocks();
}

void main(void)
{
	hardware_setup();

	audio_app_main();
}
