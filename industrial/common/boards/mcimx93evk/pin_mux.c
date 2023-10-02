/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"

/*- Used_pin:
  - {pin_num: AA11, peripheral: ENET_QOS, signal: enet_qos_mdc, pin_signal: ENET1_MDC, OD: DISABLED, DSE: X6}
  - {pin_num: AA10, peripheral: ENET_QOS, signal: enet_qos_mdio, pin_signal: ENET1_MDIO, OD: DISABLED, DSE: X6}
  - {pin_num: W11, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_td, 0', pin_signal: ENET1_TD0, OD: DISABLED, PD: DISABLED, DSE: X6}
  - {pin_num: T12, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_td, 1', pin_signal: ENET1_TD1, OD: DISABLED, PD: DISABLED, DSE: X6}
  - {pin_num: U12, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_td, 2', pin_signal: ENET1_TD2, OD: DISABLED, PD: DISABLED, DSE: X6}
  - {pin_num: V12, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_td, 3', pin_signal: ENET1_TD3, OD: DISABLED, PD: DISABLED, DSE: X6}
  - {pin_num: V10, peripheral: ENET_QOS, signal: enet_qos_rgmii_tx_ctl, pin_signal: ENET1_TX_CTL, OD: DISABLED, PD: DISABLED, DSE: X6}
  - {pin_num: U10, peripheral: ENET_QOS, signal: ccm_enet_qos_clock_generate_tx_clk, pin_signal: ENET1_TXC, OD: DISABLED, PD: DISABLED, FSEL1: FAST_SLEW_RATE, DSE: X6}
  - {pin_num: AA7, peripheral: ENET_QOS, signal: ccm_enet_qos_clock_generate_rx_clk, pin_signal: ENET1_RXC, OD: DISABLED, FSEL1: FAST_SLEW_RATE, DSE: X6}
  - {pin_num: Y8, peripheral: ENET_QOS, signal: enet_qos_rgmii_rx_ctl, pin_signal: ENET1_RX_CTL, OD: DISABLED, DSE: X6}
  - {pin_num: AA8, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_rd, 0', pin_signal: ENET1_RD0, OD: DISABLED, DSE: X6}
  - {pin_num: Y9, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_rd, 1', pin_signal: ENET1_RD1, OD: DISABLED, DSE: X6}
  - {pin_num: AA9, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_rd, 2', pin_signal: ENET1_RD2, OD: DISABLED, DSE: X6}
  - {pin_num: Y10, peripheral: ENET_QOS, signal: 'enet_qos_rgmii_rd, 3', pin_signal: ENET1_RD3, OD: DISABLED, DSE: X6}
 */
void pin_mux_enet_qos(void)
{
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_MDC__ENET_QOS_MDC, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_MDIO__ENET_QOS_MDIO, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_RD0__ENET_QOS_RGMII_RD0, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_RD1__ENET_QOS_RGMII_RD1, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_RD2__ENET_QOS_RGMII_RD2, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_RD3__ENET_QOS_RGMII_RD3, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_RXC__CCM_ENET_QOS_CLOCK_GENERATE_RX_CLK, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_RX_CTL__ENET_QOS_RGMII_RX_CTL, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_TD0__ENET_QOS_RGMII_TD0, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_TD1__ENET_QOS_RGMII_TD1, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_TD2__ENET_QOS_RGMII_TD2, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_TD3__ENET_QOS_RGMII_TD3, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_TXC__CCM_ENET_QOS_CLOCK_GENERATE_TX_CLK, 0U);
	IOMUXC_SetPinMux(IOMUXC_PAD_ENET1_TX_CTL__ENET_QOS_RGMII_TX_CTL, 0U);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_MDC__ENET_QOS_MDC,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_MDIO__ENET_QOS_MDIO,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_RD0__ENET_QOS_RGMII_RD0,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_RD1__ENET_QOS_RGMII_RD1,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_RD2__ENET_QOS_RGMII_RD2,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_RD3__ENET_QOS_RGMII_RD3,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_RXC__CCM_ENET_QOS_CLOCK_GENERATE_RX_CLK,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(3U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_RX_CTL__ENET_QOS_RGMII_RX_CTL,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U) |
						IOMUXC_PAD_PD_MASK);
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_TD0__ENET_QOS_RGMII_TD0,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U));
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_TD1__ENET_QOS_RGMII_TD1,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U));
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_TD2__ENET_QOS_RGMII_TD2,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U));
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_TD3__ENET_QOS_RGMII_TD3,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U));
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_TXC__CCM_ENET_QOS_CLOCK_GENERATE_TX_CLK,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(3U));
	IOMUXC_SetPinConfig(IOMUXC_PAD_ENET1_TX_CTL__ENET_QOS_RGMII_TX_CTL,
						IOMUXC_PAD_DSE(63U) |
						IOMUXC_PAD_FSEL1(2U));
}

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
	pin_mux_enet_qos();
	pin_mux_flexcan2();
}
