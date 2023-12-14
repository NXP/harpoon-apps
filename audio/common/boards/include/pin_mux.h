/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

void BOARD_InitPins(void);
void pin_mux_dynamic_config(bool use_audio_hat);

#endif /* _PIN_MUX_H_ */
