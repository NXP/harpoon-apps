/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CODEC_CONFIG_H_
#define _CODEC_CONFIG_H_

enum codec_id {
	CODEC_ID_HIFIBERRY,
	CODEC_ID_WM8524,
	CODEC_ID_WM8960,
	CODEC_ID_WM8962,
	CODEC_ID_CS42448
};

int32_t codec_setup(enum codec_id cid);
int32_t codec_set_format(enum codec_id cid, uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth);
int32_t codec_close(enum codec_id cid);
bool codec_is_rate_supported(uint32_t rate, bool use_audio_hat);

static inline bool is_value_in_array(uint32_t value, uint32_t array[], size_t size)
{
	for (int i = 0; i < size; i++) {
		if (array[i] == value)
			return true;
	}

	return false;
}

#endif /* _CODEC_CONFIG_H_ */
