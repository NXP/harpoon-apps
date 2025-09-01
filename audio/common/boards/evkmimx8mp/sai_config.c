/*
 * Copyright 2022-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sai.h"

#include "app_board.h"

#include "rtos_apps/audio/audio_app.h"

struct sai_active_config audio_app_sai_active_list[] = {
	{
		.sai_base = SAI5_SAI,
		.clk_id = SAI5_CLK_ID,
		.root_clk_id = SAI5_ROOT_CLK_ID,
		.masterSlave = SAI5_MASTER_SLAVE,
		.audio_pll = kCLOCK_AudioPll1Ctrl,
		.audio_pll_mul = 1,
		.audio_pll_div = 16,
		.tx_sync_mode = SAI5_TX_SYNC_MODE,
		.rx_sync_mode = SAI5_RX_SYNC_MODE,
		.msel = kSAI_BclkSourceMclkDiv,		/* MCLK1 */
		.codec_id = CODEC_ID_HIFIBERRY,
		.slot_count = DEMO_AUDIO_DATA_CHANNEL,
		.slot_size = DEMO_AUDIO_BIT_WIDTH,
		.rx_mask = SAI_DATA_MASK_NONE,
		.tx_mask = SAI_DATA_MASK_NONE,
	},
	{
		.sai_base = SAI3_SAI,
		.clk_id = SAI3_CLK_ID,
		.root_clk_id = SAI3_ROOT_CLK_ID,
		.masterSlave = SAI3_MASTER_SLAVE,
		.audio_pll = kCLOCK_AudioPll1Ctrl,
		.audio_pll_mul = 1,
		.audio_pll_div = 16,
		.tx_sync_mode = SAI3_TX_SYNC_MODE,
		.rx_sync_mode = SAI3_RX_SYNC_MODE,
		.msel = kSAI_BclkSourceMclkDiv,		/* MCLK1 */
		.codec_id = CODEC_ID_WM8960,
		.slot_count = DEMO_AUDIO_DATA_CHANNEL,
		.slot_size = DEMO_AUDIO_BIT_WIDTH,
		.rx_mask = SAI_DATA_MASK_NONE,
		.tx_mask = SAI_DATA_MASK_NONE,
	},
};

int32_t audio_app_sai_active_list_nelems = ARRAY_SIZE(audio_app_sai_active_list);

void audio_app_sai_alternate_config(bool use_audio_hat, unsigned int rate)
{
	return;
}
