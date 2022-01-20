/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SAI_CODEC_CONFIG_H_
#define _SAI_CODEC_CONFIG_H_

void codec_setup(void);
void codec_set_format(uint32_t mclk, uint32_t sample_rate, uint32_t bitwidth);
void codec_close(void);

#endif /* _SAI_CODEC_CONFIG_H_ */
