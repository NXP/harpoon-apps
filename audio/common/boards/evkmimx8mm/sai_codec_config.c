/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_codec_pcm186x_adapter.h"
#include "fsl_codec_pcm512x_adapter.h"
#include "fsl_codec_common.h"
#include "fsl_pcm186x.h"
#include "fsl_pcm512x.h"

#include "sai_codec_config.h"

#include "os/assert.h"
#include "board.h"

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

void codec_set_format(enum codec_id cid, uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth)
{
    int32_t err;

    if (cid == CODEC_ID_HIFIBERRY) {
        err = CODEC_SetFormat(&pcm512x_codec_handle, mclk, sample_rate, bitwidth);
        os_assert(err == kStatus_Success, "PCM5122 set format failed (err %d)", err);

        err = CODEC_SetFormat(&pcm186x_codec_handle, mclk, sample_rate, bitwidth);
        os_assert(err == kStatus_Success, "PCM1863 set format failed (err %d)", err);
    }
    else if (cid == CODEC_ID_WM8524) {
        err = CODEC_SetFormat(&wm8524_codec_handle, mclk, sample_rate, bitwidth);
        os_assert(err == kStatus_Success, "WM8524 set format failed (err %d)", err);
    }
    else {
        os_assert(0, "Unexpected codec id (%d)", cid);
    }
}

void codec_setup(enum codec_id cid)
{
    int32_t err;

    if (cid == CODEC_ID_HIFIBERRY) {
        /* Setup I2C clock */
        CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
        CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
        CLOCK_EnableClock(kCLOCK_I2c3);

        /* Use default setting to init ADC and DAC */
        err = CODEC_Init(&pcm512x_codec_handle, &pcm512x_codec_config);
        os_assert(err == kStatus_Success, "PCM5122 initialisation failed (err %d)", err);

        err = CODEC_Init(&pcm186x_codec_handle, &pcm186x_codec_config);
        os_assert(err == kStatus_Success, "PCM1863 initialisation failed (err %d)", err);
    }
    else if (cid == CODEC_ID_WM8524) {
        /* Use default setting to init codec */
        err = CODEC_Init(&wm8524_codec_handle, &wm8524_codec_config);
        os_assert(err == kStatus_Success, "WM8524 initialisation failed (err %d)", err);
    }
    else {
        os_assert(0, "Unexpected codec id (%d)", cid);
    }
}

void codec_close(enum codec_id cid)
{
    int32_t err;

    if (cid == CODEC_ID_HIFIBERRY) {
        err = CODEC_Deinit(&pcm512x_codec_handle);
        os_assert(err == kStatus_Success, "PCM5122 deinitialisation failed (err %d)", err);

        err = CODEC_Deinit(&pcm186x_codec_handle);
        os_assert(err == kStatus_Success, "PCM1863 deinitialisation failed (err %d)", err);
    }
    else if (cid == CODEC_ID_WM8524) {
        err = CODEC_Deinit(&wm8524_codec_handle);
        os_assert(err == kStatus_Success || err == kStatus_CODEC_NotSupport,
            "WM8524 deinitialisation failed (err %d)", err);
    }
    else {
        os_assert(0, "Unexpected codec id (%d)", cid);
    }
}
