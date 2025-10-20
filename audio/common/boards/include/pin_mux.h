/*
 * Copyright 2022-2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include <stdbool.h>

void BOARD_InitPins(void);
void BOARD_pin_mux_dynamic_config(bool use_audio_hat);

#endif /* _PIN_MUX_H_ */
