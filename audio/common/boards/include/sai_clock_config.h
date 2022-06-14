/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _SAI_CLOCK_CONFIG_H_
#define _SAI_CLOCK_CONFIG_H_

void sai_clock_setup(void);
uint32_t get_sai_clock_root(uint32_t id);
uint32_t sai_select_audio_pll_mux(int sai_id, int srate);

#endif /* _SAI_CLOCK_CONFIG_H_ */
