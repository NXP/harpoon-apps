/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hal_pinctrl.h"
#include "hal_pinctrl_platform.h"

void pin_mux_enetc(void)
{
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD3__ETH0_RGMII_TD3, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD2__ETH0_RGMII_TD2, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD1__ETH0_RGMII_TD1, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD0__ETH0_RGMII_TD0, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TX_CTL__ETH0_RGMII_TX_CTL, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TXC__ETH0_RGMII_TX_CLK, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RX_CTL__ETH0_RGMII_RX_CTL, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RXC__ETH0_RGMII_RX_CLK, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD0__ETH0_RGMII_RD0, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD1__ETH0_RGMII_RD1, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD2__ETH0_RGMII_RD2, 0U);
	HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD3__ETH0_RGMII_RD3, 0U);

	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD3__ETH0_RGMII_TD3, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD2__ETH0_RGMII_TD2, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD1__ETH0_RGMII_TD1, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TD0__ETH0_RGMII_TD0, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TX_CTL__ETH0_RGMII_TX_CTL, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_TXC__ETH0_RGMII_TX_CLK, 0x5fe);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RX_CTL__ETH0_RGMII_RX_CTL, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RXC__ETH0_RGMII_RX_CLK, 0x5fe);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD0__ETH0_RGMII_RD0, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD1__ETH0_RGMII_RD1, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD2__ETH0_RGMII_RD2, 0x57e);
	HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_ENET1_RD3__ETH0_RGMII_RD3, 0x57e);
}

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
