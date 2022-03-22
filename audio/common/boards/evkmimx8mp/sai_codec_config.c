/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef CODEC_WM8960
#include "fsl_codec_adapter.h"
#elif defined(CODEC_HIFIBERRY)
#include "fsl_codec_pcm186x_adapter.h"
#include "fsl_codec_pcm512x_adapter.h"
#endif
#include "fsl_codec_common.h"
#include "fsl_iomuxc.h"
#ifdef CODEC_WM8960
#include "fsl_wm8960.h"
#elif defined(CODEC_HIFIBERRY)
#include "fsl_pcm512x.h"
#include "fsl_pcm186x.h"
#endif

#include "os/assert.h"

#include "board.h"

/*I.MX8MP EVK onboard Codec is WM8960 */
#ifdef CODEC_WM8960
static codec_handle_t codec_handle;

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
		.sampleRate = kWM8960_AudioSampleRate44100Hz,
		.bitWidth   = kWM8960_AudioBitWidth32bit
	},
	.master_slave    = false,
};
static codec_config_t sai_codec_config = {.codecDevType = kCODEC_WM8960,
	.codecDevConfig = &wm8960Config};

void codec_set_format(uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth)
{
	int32_t err;

	err = CODEC_SetFormat(&codec_handle, mclk, sample_rate, bitwidth);
	os_assert(err == kStatus_Success, "WM8960 set format failed (err %d)", err);
}

void codec_setup(void)
{
	int32_t err;

	/* setup clock */
	CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
	CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
	CLOCK_EnableClock(kCLOCK_I2c3);

	/* Use default setting to init codec */
	err = CODEC_Init(&codec_handle, &sai_codec_config);
	os_assert(err == kStatus_Success, "Codec initialization failed (err %d)", err);
}

void codec_close(void)
{
	int32_t err;

	err = CODEC_Deinit(&codec_handle);
	if (err != kStatus_Success) {
		os_assert(false, "Codec deinitialization failed (err %d)", err);
	}
}
#elif defined(CODEC_HIFIBERRY)
/*
 * Two Codecs are on HiFiBerry Board:
 * ADC Codec is PCM186x:
 * DAC Codec is PCM512x:
 */
static codec_handle_t dac_handle;
static codec_handle_t adc_handle;

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
	.gpio_osc48 = PCM512X_GPIO_OSC48,
};
static codec_config_t dac_config = {
	.codecDevType = kCODEC_PCM512X,
	.codecDevConfig = &pcm512xConfig,
};

static pcm186x_config_t pcm186xConfig = {
    .i2cConfig = {
        .codecI2CInstance = BOARD_CODEC_I2C_INSTANCE,
        .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ
    },
    .slaveAddress = PCM186X_I2C_ADDR,
    .format = {
        .mclk_HZ = 24576000U,
        .bitWidth   = kPCM186x_AudioBitWidth32bit
    },
    .gpio_led   = PCM186X_GPIO_LED,
};
static codec_config_t adc_config = {
    .codecDevType = kCODEC_PCM186X,
    .codecDevConfig = &pcm186xConfig,
};

void codec_set_format(uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth)
{
	int32_t err;

	/* In general, ADC and DAC will use the same configuration parameters */
	err = CODEC_SetFormat(&dac_handle, mclk, sample_rate, bitwidth);
	os_assert(err == kStatus_Success, "DAC set format failed (err %d)", err);

	err = CODEC_SetFormat(&adc_handle, mclk, sample_rate, bitwidth);
	os_assert(err == kStatus_Success, "ADC set format failed (err %d)", err);
}

void codec_setup(void)
{
	int32_t err;

	/* setup clock */
	CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
	CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
	CLOCK_EnableClock(kCLOCK_I2c3);

	/* Use default setting to init codec */
	err = CODEC_Init(&dac_handle, &dac_config);
	os_assert(err == kStatus_Success, "DAC initialization failed (err %d)", err);

	err = CODEC_Init(&adc_handle, &adc_config);
	os_assert(err == kStatus_Success, "ADC initialization failed (err %d)", err);
}

void codec_close(void)
{
	int32_t err;

	err = CODEC_Deinit(&dac_handle);
	os_assert(err == kStatus_Success, "DAC deinitialization failed (err %d)", err);

	err = CODEC_Deinit(&adc_handle);
	os_assert(err == kStatus_Success, "ADC deinitialization failed (err %d)", err);
}
#endif
