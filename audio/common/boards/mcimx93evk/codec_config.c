/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_codec_common.h"
#include "codec_config.h"

#include "os/assert.h"
#include "app_board.h"
#include "hlog.h"

static codec_handle_t wm8962_codec_handle;

wm8962_config_t wm8962Config = {
	.i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
	.route =
		{
			.enableLoopBack            = false,
			.leftInputPGASource        = kWM8962_InputPGASourceInput1,
			.leftInputMixerSource      = kWM8962_InputMixerSourceInputPGA,
			.rightInputPGASource       = kWM8962_InputPGASourceInput3,
			.rightInputMixerSource     = kWM8962_InputMixerSourceInputPGA,
			.leftHeadphoneMixerSource  = kWM8962_OutputMixerDisabled,
			.leftHeadphonePGASource    = kWM8962_OutputPGASourceDAC,
			.rightHeadphoneMixerSource = kWM8962_OutputMixerDisabled,
			.rightHeadphonePGASource   = kWM8962_OutputPGASourceDAC,
		},
	.slaveAddress = WM8962_I2C_ADDR,
	.bus          = kWM8962_BusI2S,
	.format       = {.sampleRate = kWM8962_AudioSampleRate48KHz, .bitWidth = kWM8962_AudioBitWidth16bit},
	.fllClock =
		{
			.fllClockSource        = kWM8962_FLLClkSourceMCLK,
			.fllReferenceClockFreq = 12288000U,
			.fllOutputFreq         = 12288000U,
		},
	.sysclkSource = kWM8962_SysClkSourceMclk,
	.masterSlave  = false, /* sai use as master mode, so codec as slave mode */
};

static codec_config_t wm8962_codec_config = {
	.codecDevType = kCODEC_WM8962,
	.codecDevConfig = &wm8962Config
};

int32_t codec_set_format(enum codec_id cid, uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth)
{
	int32_t err;

	if (cid == CODEC_ID_WM8962) {
		err = CODEC_SetFormat(&wm8962_codec_handle, mclk, sample_rate, bitwidth);
		if (err != kStatus_Success) {
			log_warn("WM8962 set format failed: sample rate %d not supported (err %d)\n",sample_rate, err);
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
	int32_t err;

	if (cid == CODEC_ID_WM8962) {
		/* Setup I2C clock */
		const clock_root_config_t lpi2cClkCfg = {
			.clockOff = false,
			.mux = 0, // 24MHz oscillator source
			.div = 1
		};

		CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &lpi2cClkCfg);
		CLOCK_EnableClock(kCLOCK_Lpi2c1);

		wm8962Config.i2cConfig.codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ;
		wm8962Config.format.mclk_HZ                = WM8962_SAI_CLK_FREQ;

		/* Use default setting to init codec */
		err = CODEC_Init(&wm8962_codec_handle, &wm8962_codec_config);
		if ((err != kStatus_Success) && (err != kStatus_CODEC_NotSupport)) {
			log_err("WM8962 initialisation failed (err %d)\n", err);
			goto end;
		}

		/* Set volume on both channels */
		err = CODEC_SetVolume(&wm8962_codec_handle, (kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight), 75);
		if (err != kStatus_Success) {
			log_err("WM8962 set volume failed (err %d)\n", err);
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

	if (cid == CODEC_ID_WM8962) {
		err = CODEC_Deinit(&wm8962_codec_handle);
		if (err != kStatus_Success) {
			log_err("WM8962 deinitialisation failed (err %d)\n", err);
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
