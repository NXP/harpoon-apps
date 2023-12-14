/*
 * Copyright 2021-2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_codec_pcm186x_adapter.h"
#include "fsl_codec_pcm512x_adapter.h"
#include "fsl_codec_common.h"
#include "fsl_pcm186x.h"
#include "fsl_pcm512x.h"

#include "codec_config.h"

#include "os/assert.h"
#include "app_board.h"
#include "hlog.h"

static codec_handle_t wm8524_codec_handle;
static codec_handle_t pcm512x_codec_handle;
static codec_handle_t pcm186x_codec_handle;

static void codec_wm8524_setmute(uint32_t mute)
{
}

static wm8524_config_t wm8524Config = {
	.setMute = codec_wm8524_setmute,
};

static codec_config_t wm8524_codec_config = {
	.codecDevType = kCODEC_WM8524,
	.codecDevConfig = &wm8524Config
};

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

static codec_config_t pcm512x_codec_config = {
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

static codec_config_t pcm186x_codec_config = {
	.codecDevType = kCODEC_PCM186X,
	.codecDevConfig = &pcm186xConfig,
};

int32_t codec_set_format(enum codec_id cid, uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth)
{
	int32_t err;

	if (cid == CODEC_ID_HIFIBERRY) {
		err = CODEC_SetFormat(&pcm512x_codec_handle, mclk, sample_rate, bitwidth);
		if (err != kStatus_Success) {
			log_err("PCM5122 set format failed (err %d)\n", err);
			goto end;
		}

		err = CODEC_SetFormat(&pcm186x_codec_handle, mclk, sample_rate, bitwidth);
		if (err != kStatus_Success) {
			log_err("PCM1863 set format failed (err %d)\n", err);
			goto end;
		}
	}
	else if (cid == CODEC_ID_WM8524) {
		err = CODEC_SetFormat(&wm8524_codec_handle, mclk, sample_rate, bitwidth);
		if (err != kStatus_Success) {
			log_err("WM8524 set format failed (err %d)\n", err);
			goto end;
		}
	}
	else {
		err = -1;
		os_assert(0, "Unexpected codec id (%d)", cid);
	}

end:
	return err;
}

int32_t codec_setup(enum codec_id cid)
{
	int32_t err = 0;

	if (cid == CODEC_ID_HIFIBERRY) {
		/* Setup I2C clock */
		CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
		CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
		CLOCK_EnableClock(kCLOCK_I2c3);

		/* Use default setting to init ADC and DAC */
		err = CODEC_Init(&pcm512x_codec_handle, &pcm512x_codec_config);
		if (err != kStatus_Success) {
			log_err("PCM5122 initialisation failed (err %d)\n", err);
			goto end;
		}

		err = CODEC_Init(&pcm186x_codec_handle, &pcm186x_codec_config);
		if (err != kStatus_Success) {
			log_err("PCM1863 initialisation failed (err %d)\n", err);
			goto end;
		}
	}
	else if (cid == CODEC_ID_WM8524) {
		/* Use default setting to init codec */
		err = CODEC_Init(&wm8524_codec_handle, &wm8524_codec_config);
		if ((err != kStatus_Success) && (err != kStatus_CODEC_NotSupport)) {
			log_err("WM8524 initialisation failed (err %d)\n", err);
			goto end;
		}
		err = kStatus_Success;
	}
	else {
		err = -1;
		os_assert(0, "Unexpected codec id (%d)", cid);
	}

end:
	return err;
}

int32_t codec_close(enum codec_id cid)
{
	int32_t err;

	if (cid == CODEC_ID_HIFIBERRY) {
		err = CODEC_Deinit(&pcm512x_codec_handle);
		if (err != kStatus_Success) {
			log_err("PCM5122 deinitialisation failed (err %d)\n", err);
			goto end;
		}

		err = CODEC_Deinit(&pcm186x_codec_handle);
		if (err != kStatus_Success) {
			log_err("PCM1863 deinitialisation failed (err %d)\n", err);
			goto end;
		}
	}
	else if (cid == CODEC_ID_WM8524) {
		err = CODEC_Deinit(&wm8524_codec_handle);
		if ((err != kStatus_Success) && (err != kStatus_CODEC_NotSupport)) {
			log_err("WM8524 deinitialisation failed (err %d)\n", err);
			goto end;
		}
	}
	else {
		err = -1;
		os_assert(0, "Unexpected codec id (%d)", cid);
	}

end:
	return err;
}

bool codec_is_rate_supported(uint32_t rate, bool use_audio_hat)
{
	uint32_t supported_rates[] = SUPPORTED_RATES;

	return is_value_in_array(rate, supported_rates, ARRAY_SIZE(supported_rates));
}
