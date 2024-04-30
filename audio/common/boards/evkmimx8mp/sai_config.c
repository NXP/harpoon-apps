/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sai.h"

#include "app_board.h"
#include "codec_config.h"
#include "sai_config.h"

struct sai_active_config sai_active_list[] = {
	{
		.sai_base = SAI5_SAI,
		.masterSlave = SAI5_MASTER_SLAVE,
		.audio_pll = kCLOCK_AudioPll1Ctrl,
		.audio_pll_mul = 1,
		.audio_pll_div = 16,
		.tx_sync_mode = SAI5_TX_SYNC_MODE,
		.rx_sync_mode = SAI5_RX_SYNC_MODE,
		.msel = kSAI_BclkSourceMclkDiv,		/* MCLK1 */
		.cid = CODEC_ID_HIFIBERRY,
		.slot_count = DEMO_AUDIO_DATA_CHANNEL,
		.slot_size = DEMO_AUDIO_BIT_WIDTH,
		.rx_mask = SAI_DATA_MASK_NONE,
		.tx_mask = SAI_DATA_MASK_NONE,
	},
	{
		.sai_base = SAI3_SAI,
		.masterSlave = SAI3_MASTER_SLAVE,
		.audio_pll = kCLOCK_AudioPll1Ctrl,
		.audio_pll_mul = 1,
		.audio_pll_div = 16,
		.tx_sync_mode = SAI3_TX_SYNC_MODE,
		.rx_sync_mode = SAI3_RX_SYNC_MODE,
		.msel = kSAI_BclkSourceMclkDiv,		/* MCLK1 */
		.cid = CODEC_ID_WM8960,
		.slot_count = DEMO_AUDIO_DATA_CHANNEL,
		.slot_size = DEMO_AUDIO_BIT_WIDTH,
		.rx_mask = SAI_DATA_MASK_NONE,
		.tx_mask = SAI_DATA_MASK_NONE,
	},
};

int32_t sai_active_list_nelems = ARRAY_SIZE(sai_active_list);

void sai_set_audio_hat_codec(bool use_audio_hat, unsigned int rate)
{
	return;
}
