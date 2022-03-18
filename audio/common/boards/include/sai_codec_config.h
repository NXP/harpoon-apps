/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SAI_CODEC_CONFIG_H_
#define _SAI_CODEC_CONFIG_H_

enum codec_id {
    CODEC_ID_HIFIBERRY,
    CODEC_ID_WM8524,
    CODEC_ID_WM8960
};

void codec_setup(enum codec_id cid);
void codec_set_format(enum codec_id cid, uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth);
void codec_close(enum codec_id cid);

#endif /* _SAI_CODEC_CONFIG_H_ */
