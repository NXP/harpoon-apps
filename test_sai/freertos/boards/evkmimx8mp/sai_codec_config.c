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
#ifdef CODEC_WM8960
#include "fsl_wm8960.h"
#elif defined(CODEC_HIFIBERRY)
#include "fsl_pcm512x.h"
#endif

#include "os/assert.h"

#include "board.h"

static codec_handle_t	sai_codec_handle;

#ifdef CODEC_WM8960
static wm8960_config_t wm8960Config = {
	.i2cConfig = {
		.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE,
		.codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
	.route     = kWM8960_RoutePlaybackandRecord,
	.leftInputSource = kWM8960_InputDifferentialMicInput3,
	.playSource      = kWM8960_PlaySourceDAC,
	.slaveAddress    = WM8960_I2C_ADDR,
	.bus             = kWM8960_BusI2S,
	.format          = {
		.mclk_HZ    = 24576000U,
		.sampleRate = kWM8960_AudioSampleRate16KHz,
		.bitWidth   = kWM8960_AudioBitWidth16bit
	},
	.master_slave    = false,
};
static codec_config_t sai_codec_config = {.codecDevType = kCODEC_WM8960,
	.codecDevConfig = &wm8960Config};

void codec_set_format(void)
{
}

#elif defined(CODEC_HIFIBERRY)
static pcm512x_config_t pcm512xConfig = {
	.i2cConfig = {
		.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE,
		.codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ
	},
	.slaveAddress = PCM512X_I2C_ADDR,
	.format = {
		.mclk_HZ = 24576000U,
		.sampleRate = kPCM512x_AudioSampleRate44100Hz,
		.bitWidth   = kPCM512x_AudioBitWidth32bit
	},
	.gpio_led   = PCM512X_GPIO_LED,
	.gpio_osc44 = PCM512X_GPIO_OSC44,
	.gpio_osc48 = PCM512X_GPIO_OSC48
};
static codec_config_t sai_codec_config = {
	.codecDevType = kCODEC_PCM512X,
	.codecDevConfig = &pcm512xConfig,
};

void codec_set_format(void)
{
	int32_t err;
	uint32_t mclk, sample_rate, bitwidth;

	mclk = ((pcm512x_config_t *)(sai_codec_config.codecDevConfig))->format.mclk_HZ;
	sample_rate = ((pcm512x_config_t *)(sai_codec_config.codecDevConfig))->format.sampleRate;
	bitwidth = ((pcm512x_config_t *)(sai_codec_config.codecDevConfig))->format.bitWidth;

	err = CODEC_SetFormat(&sai_codec_handle, mclk, sample_rate, bitwidth);
	if (err != kStatus_Success) {
		os_assert(false, "Codec set format failed (err %d)", err);
	}
}
#endif

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

void codec_close(void)
{
	int32_t err;

	err = CODEC_Deinit(&sai_codec_handle);
	if (err != kStatus_Success) {
		os_assert(false, "Codec deinitialization failed (err %d)", err);
	}
}
