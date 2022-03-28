/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CODEC_CONFIG_H_
#define _CODEC_CONFIG_H_

enum codec_id {
    CODEC_ID_HIFIBERRY,
    CODEC_ID_WM8524,
    CODEC_ID_WM8960
};

int32_t codec_setup(enum codec_id cid);
int32_t codec_set_format(enum codec_id cid, uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth);
int32_t codec_close(enum codec_id cid);

#endif /* _CODEC_CONFIG_H_ */
