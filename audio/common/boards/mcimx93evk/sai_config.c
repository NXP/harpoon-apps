/*
 * Copyright 2023 NXP
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
		.sai_base = SAI3_SAI,
		.masterSlave = SAI3_MASTER_SLAVE,
		.audio_pll = kCLOCK_AudioPll1Out,
		.audio_pll_mul = 1,
		.audio_pll_div = 32,
		.tx_sync_mode = SAI3_TX_SYNC_MODE,
		.rx_sync_mode = SAI3_RX_SYNC_MODE,
		.msel = kSAI_BclkSourceMclkDiv,		/* MCLK1 */
		.cid = CODEC_ID_WM8962,
	},
};

int32_t sai_active_list_nelems = ARRAY_SIZE(sai_active_list);