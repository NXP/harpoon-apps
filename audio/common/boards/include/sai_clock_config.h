/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _SAI_CLOCK_CONFIG_H_
#define _SAI_CLOCK_CONFIG_H_

#include <stdint.h>

void sai_clock_setup(void);
uint32_t sai_select_audio_pll_mux(int sai_id, int srate);
uint32_t get_sai_clock_freq(unsigned int sai_active_index);

#endif /* _SAI_CLOCK_CONFIG_H_ */
