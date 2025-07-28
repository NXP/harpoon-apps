/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sai.h"

#include "app_board.h"
#include "rtos_abstraction_layer.h"

#include "rtos_apps/audio/audio_app.h"

struct sai_active_config audio_app_sai_active_list[] = {
	{
		.sai_base = SAI3_SAI,
		.clk_id = SAI3_CLK_ID,
		.root_clk_id = SAI3_ROOT_CLK_ID,
		.masterSlave = SAI3_MASTER_SLAVE,
		.audio_pll = kCLOCK_AudioPll1Out,
		.audio_pll_mul = 1,
		.audio_pll_div = 32,
		.tx_sync_mode = SAI3_TX_SYNC_MODE,
		.rx_sync_mode = SAI3_CS42448_RX_SYNC_MODE,
		.msel = kSAI_BclkSourceMclkDiv,		/* MCLK1 */
		.cid = CODEC_ID_CS42448,
		.slot_count = DEMO_AUDIO_DATA_CHANNEL,
		.slot_size = DEMO_AUDIO_BIT_WIDTH,
		.rx_mask = SAI_DATA_MASK_NONE,
		.tx_mask = SAI_DATA_MASK_NONE,
	},
};

int32_t audio_app_sai_active_list_nelems = ARRAY_SIZE(audio_app_sai_active_list);

void audio_app_sai_alternate_config(bool use_audio_hat, unsigned int rate)
{
	if (use_audio_hat) {
		/* MX93AUD_HAT connected to the first SAI: SAI3 */
		audio_app_sai_active_list[0].cid = CODEC_ID_CS42448;
		audio_app_sai_active_list[0].rx_sync_mode = SAI3_CS42448_RX_SYNC_MODE;
		audio_app_sai_active_list[0].slot_count = DEMO_MX93AUDHAT_AUDIO_DATA_CHANNEL;
		audio_app_sai_active_list[0].slot_size = DEMO_MX93AUDHAT_AUDIO_BIT_WIDTH;
		audio_app_sai_active_list[0].rx_mask = 0xFFFFFFC0U; /* MX93AUD-HAT has 6 input channels */
		audio_app_sai_active_list[0].tx_mask = SAI_DATA_MASK_NONE;

		switch (rate) {
		case 48000:
			audio_app_sai_active_list[0].audio_pll_div = 32; /* MCLK = 12288000 Hz */
			break;
		case 96000:
			audio_app_sai_active_list[0].audio_pll_div = 16; /* MCLK = 24576000 Hz */
			break;
		case 192000:
			audio_app_sai_active_list[0].audio_pll_div = 8; /* MCLK = 49152000 Hz */
			break;
		default:
			rtos_assert(false, "Unsupported Sample Rate for MX93-AUDHAT\n");
			break;
		}
	} else {
		/* On board codec connected to the first SAI: SAI3 */
		audio_app_sai_active_list[0].cid = CODEC_ID_WM8962;
		audio_app_sai_active_list[0].rx_sync_mode = SAI3_WM8962_RX_SYNC_MODE;
		audio_app_sai_active_list[0].slot_count = DEMO_AUDIO_DATA_CHANNEL;
		audio_app_sai_active_list[0].slot_size = DEMO_AUDIO_BIT_WIDTH;
		audio_app_sai_active_list[0].rx_mask = SAI_DATA_MASK_NONE;
		audio_app_sai_active_list[0].tx_mask = SAI_DATA_MASK_NONE;
	}
}
