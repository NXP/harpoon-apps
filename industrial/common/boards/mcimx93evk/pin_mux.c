/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"

/*
- Used pin:
  - {pin_num: W21, peripheral: CAN2, signal: can_rx, pin_signal: GPIO_IO27, HYS: DISABLED}
  - {pin_num: V21, peripheral: CAN2, signal: can_tx, pin_signal: GPIO_IO25, HYS: DISABLED}
 */
void pin_mux_flexcan2(void) {
	IOMUXC_SetPinMux(IOMUXC_PAD_GPIO_IO25__CAN2_TX, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_GPIO_IO27__CAN2_RX, 0U);

	IOMUXC_SetPinConfig(IOMUXC_PAD_GPIO_IO25__CAN2_TX,
						IOMUXC_PAD_DSE(15U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_GPIO_IO27__CAN2_RX,
						IOMUXC_PAD_DSE(15U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
}

void BOARD_InitPins(void)
{
	pin_mux_flexcan2();
}
