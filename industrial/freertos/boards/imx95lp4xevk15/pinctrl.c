/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hal_pinctrl.h"
#include "hal_pinctrl_platform.h"

/*
 * - Used pin:
 *   - {pin_num: V21, peripheral: CAN2, signal: can_tx, pin_signal: GPIO_IO25, HYS: DISABLED}
 *   - {pin_num: W21, peripheral: CAN2, signal: can_rx, pin_signal: GPIO_IO27, HYS: DISABLED}
 */
static inline void pin_mux_flexcan2(void)
{
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO25__CAN2_TX, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO27__CAN2_RX, 0U);

	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO25__CAN2_TX,
			     HAL_PINCTRL_PLATFORM_IOMUXC_PAD_DSE(15U)  |
			     HAL_PINCTRL_PLATFORM_IOMUXC_PAD_FSEL1(2U) |
			     HAL_PINCTRL_PLATFORM_IOMUXC_PAD_PD_MASK);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO27__CAN2_RX,
			     HAL_PINCTRL_PLATFORM_IOMUXC_PAD_DSE(15U)  |
			     HAL_PINCTRL_PLATFORM_IOMUXC_PAD_FSEL1(2U) |
			     HAL_PINCTRL_PLATFORM_IOMUXC_PAD_PD_MASK);
}

void pin_mux_flexcan(void)
{
	pin_mux_flexcan2();
}
