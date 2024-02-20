/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_codec_common.h"
#include "fsl_sai.h"
#include "fsl_cs42448.h"
#include "fsl_rgpio.h"

#include "os/assert.h"

#include "app_board.h"
#include "codec_config.h"
#include "hlog.h"

#define WM8962_THREED1					0x10CU
#define WM8962_THREED1_ADCMONOMIX_MASK	0x40U
#define WM8962_THREED1_ADCMONOMIX_SHIFT	0x06U

static codec_handle_t cs42448_codec_handle;
static codec_handle_t wm8962_codec_handle;

static void codec_cs42448_reset(bool state)
{
	if (state)
		RGPIO_PinWrite(CS42448_RESET_GPIO, CS42448_RESET_GPIO_PIN, 1U);
	else
		RGPIO_PinWrite(CS42448_RESET_GPIO, CS42448_RESET_GPIO_PIN, 0U);
}

static cs42448_config_t cs42448Config = {
	.DACMode	= kCS42448_ModeSlave,
	.ADCMode	= kCS42448_ModeSlave,
	.reset		= codec_cs42448_reset,
	.master		= false,
	.format		= {
		.mclk_HZ	= 12288000U,
		.sampleRate	= 48000U,
		.bitWidth	= 24U
	},
	.bus			= kCS42448_BusTDM,
	.slaveAddress	= CS42448_I2C_ADDR,
};

static codec_config_t cs42448_codec_config = {
	.codecDevType = kCODEC_CS42448,
	.codecDevConfig = &cs42448Config
};

static wm8962_config_t wm8962Config = {
	.i2cConfig = {.codecI2CInstance = WM8962_I2C_INSTANCE},
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

static int32_t codec_cs42448_setup()
{
	int32_t err = kStatus_Success;

	/* Setup I2C clock */
	const clock_root_config_t lpi2cClkCfg = {
		.clockOff = false,
		.mux = 0, // 24MHz oscillator source
		.div = 1
	};

	/* Define the init structure for the output LED pin*/
	rgpio_pin_config_t reset_pin_config = {
		kRGPIO_DigitalOutput,
		1, /* Set default to high, as the reset is active low */
	};

	/* Init output LED GPIO. */
	CLOCK_EnableClock(CS42448_RESET_GPIO_CLOCK_GATE);
	RGPIO_PinInit(CS42448_RESET_GPIO, CS42448_RESET_GPIO_PIN, &reset_pin_config);

	CLOCK_SetRootClock(kCLOCK_Root_Lpi2c4, &lpi2cClkCfg);
	CLOCK_EnableClock(kCLOCK_Lpi2c4);

	cs42448Config.i2cConfig.codecI2CInstance = CS42448_I2C_INSTANCE;
	cs42448Config.i2cConfig.codecI2CSourceClock = CS42448_I2C_CLOCK_FREQ;

	err = CODEC_Init(&cs42448_codec_handle, &cs42448_codec_config);
	if ((err != kStatus_Success) && (err != kStatus_CODEC_NotSupport)) {
		log_err("CS42448 initialization failed (err %d)\n", err);
		goto end;
	}

	err = CODEC_SetVolume(&cs42448_codec_handle, (kCODEC_VolumeLeft0 | kCODEC_VolumeRight0 |
						      kCODEC_VolumeLeft1 | kCODEC_VolumeRight1 |
						      kCODEC_VolumeLeft2 | kCODEC_VolumeRight2), 100);
	if (err != kStatus_Success) {
		log_err("CS42448 set volume failed (err %d)\n", err);
		goto end;
	}

end:
	return err;
}

static int32_t codec_wm8962_setup()
{
	int32_t err = kStatus_Success;

	const clock_root_config_t lpi2cClkCfg = {
		.clockOff = false,
		.mux = 0, // 24MHz oscillator source
		.div = 1
	};

	CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &lpi2cClkCfg);
	CLOCK_EnableClock(kCLOCK_Lpi2c1);

	/* select MCLK direction(Enable MCLK clock):
	* The WM8962 codec won't output any sound unless we enable the SAI master clock first.
	*/
	sai_master_clock_t saiMasterCfg = {
		.mclkOutputEnable = true,
	};

	saiMasterCfg.mclkSourceClkHz = SAI3_CLK_FREQ;            /* setup source clock for MCLK */
	saiMasterCfg.mclkHz          = saiMasterCfg.mclkSourceClkHz; /* setup target clock of MCLK */
	SAI_SetMasterClockConfig(SAI3_SAI, &saiMasterCfg);

	wm8962Config.i2cConfig.codecI2CSourceClock = WM8962_I2C_CLOCK_FREQ;
	wm8962Config.format.mclk_HZ                = SAI3_CLK_FREQ;

	/* Use default setting to init codec */
	err = CODEC_Init(&wm8962_codec_handle, &wm8962_codec_config);
	if ((err != kStatus_Success) && (err != kStatus_CODEC_NotSupport)) {
		log_err("WM8962 initialization failed (err %d)\n", err);
		goto end;
	}

	/* Set volume on both channels */
	err = CODEC_SetVolume(&wm8962_codec_handle, (kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight), 75);
	if (err != kStatus_Success) {
		log_err("WM8962 set volume failed (err %d)\n", err);
		goto end;
	}

	/* To have a stereo signal from a mono channel microphone on the EVK:
	* Enable ADC Monomix to mix both Right and Left ADC signals
	*/
	wm8962_handle_t *devHandle;
	devHandle = (wm8962_handle_t *)((uintptr_t)(((codec_handle_t *)&wm8962_codec_handle)->codecDevHandle));

	err = WM8962_ModifyReg(devHandle, WM8962_THREED1, WM8962_THREED1_ADCMONOMIX_MASK, 1 << WM8962_THREED1_ADCMONOMIX_SHIFT);
	if (err != kStatus_Success) {
		log_err("WM8962 enable ADC Monomix failed (err %d)\n", err);
		goto end;
	}

end:
	return err;
}

int32_t codec_set_format(enum codec_id cid,uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth)
{
	int32_t err = kStatus_Success;

	if (cid == CODEC_ID_WM8962) {
		err = CODEC_SetFormat(&wm8962_codec_handle, mclk, sample_rate, bitwidth);
		if (err != kStatus_Success) {
			log_warn("WM8962 codec set format failed: sample rate %d not supported (err %d)\n",sample_rate, err);
			goto end;
		}
	} else if (cid == CODEC_ID_CS42448) {
		err = CODEC_SetFormat(&cs42448_codec_handle, mclk, sample_rate, bitwidth);
		if (err != kStatus_Success) {
			log_warn("CS42448 codec set format failed: sample rate %d not supported (err %d)\n",sample_rate, err);
			goto end;
		}
	} else {
		err = -1;
		os_assert(0, "Unexpected codec id (%d)", cid);
		goto end;
	}

end:
	return err;
}

int32_t codec_setup(enum codec_id cid)
{
	int32_t err;

	if (cid == CODEC_ID_WM8962) {
		err = codec_wm8962_setup();
	} else if (cid == CODEC_ID_CS42448) {
		err = codec_cs42448_setup();
	} else {
		err = -1;
		os_assert(0, "Unexpected codec id (%d)", cid);
		goto end;
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
			log_err("WM8962 deinitialization failed (err %d)\n", err);
			goto end;
		}
	} else if (cid == CODEC_ID_CS42448) {
		err = CODEC_Deinit(&cs42448_codec_handle);
		if (err != kStatus_Success) {
			log_err("CS42448 deinitialization failed (err %d)\n", err);
			goto end;
		}
	} else {
		err = -1;
		os_assert(0, "Unexpected codec id (%d)", cid);
		goto end;
	}

end:
	return err;
}

bool codec_is_rate_supported(uint32_t rate, bool use_audio_hat)
{
	uint32_t supported_rates_cs42448[] = CS42448_SUPPORTED_RATES;
	uint32_t supported_rates_wm8962[] = WM8962_SUPPORTED_RATES;
	bool ret;

	if (use_audio_hat)
		ret = is_value_in_array(rate, supported_rates_cs42448, ARRAY_SIZE(supported_rates_cs42448));
	else
		ret = is_value_in_array(rate, supported_rates_wm8962, ARRAY_SIZE(supported_rates_wm8962));

	return ret;
}