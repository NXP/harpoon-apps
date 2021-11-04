/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "fsl_codec_adapter.h"
#include "fsl_codec_common.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_wm8960.h"

#include "os/assert.h"

#include "board.h"

static codec_handle_t	sai_codec_handle;

static wm8960_config_t wm8960Config = {
	.i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE,
		.codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
	.route     = kWM8960_RoutePlaybackandRecord,
	.leftInputSource = kWM8960_InputDifferentialMicInput3,
	.playSource      = kWM8960_PlaySourceDAC,
	.slaveAddress    = WM8960_I2C_ADDR,
	.bus             = kWM8960_BusI2S,
	.format          = {.mclk_HZ    = 24576000U,
		.sampleRate = kWM8960_AudioSampleRate16KHz,
		.bitWidth   = kWM8960_AudioBitWidth16bit},
	.master_slave    = false,
};
static codec_config_t sai_codec_config = {.codecDevType = kCODEC_WM8960,
	.codecDevConfig = &wm8960Config};

void codec_setup(void)
{
	int32_t err;

	/* setup clock */
	CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
	CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
	CLOCK_EnableClock(kCLOCK_I2c3);

	/* Use default setting to init codec */
	err = CODEC_Init(&sai_codec_handle, &sai_codec_config);
	if (err != kStatus_Success) {
		os_assert(false, "Codec initialization failed (err %d)", err);
	}
}
