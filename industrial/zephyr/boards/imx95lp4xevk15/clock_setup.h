/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLOCK_SETUP_H_
#define _CLOCK_SETUP_H_

#include <stdint.h>

void BOARD_TpmClockSetup(void);
void clock_setup_flexcan(void);
void clock_setup_enetc(void);
int clock_get_flexcan_clock(uint32_t *rate);
uint32_t clock_get_mdio_clock(void);
uint32_t clock_get_netc_timer_clock(void);
uint32_t clock_get_tpm_clock(void *base);

#endif /* _CLOCK_SETUP_H_ */
