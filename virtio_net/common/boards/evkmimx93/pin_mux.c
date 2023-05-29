/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

static void pin_mux_lpuart2(void)
{
    IOMUXC_SetPinMux(IOMUXC_PAD_UART2_RXD__LPUART2_RX, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_UART2_TXD__LPUART2_TX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_PAD_UART2_RXD__LPUART2_RX,
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_UART2_TXD__LPUART2_TX,
                        IOMUXC_PAD_DSE(15U));
}

static void pin_mux_enet(void)
{
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_MDC__ENET1_MDC, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_MDIO__ENET1_MDIO, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_RD0__ENET1_RGMII_RD0, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_RD1__ENET1_RGMII_RD1, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_RD2__ENET1_RGMII_RD2, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_RD3__ENET1_RGMII_RD3, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_RXC__ENET1_RGMII_RXC, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_RX_CTL__ENET1_RGMII_RX_CTL, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_TD0__ENET1_RGMII_TD0, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_TD1__ENET1_RGMII_TD1, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_TD2__ENET1_RGMII_TD2, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_TD3__ENET1_RGMII_TD3, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_TXC__ENET1_RGMII_TXC, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_ENET2_TX_CTL__ENET1_RGMII_TX_CTL, 0U);

    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_MDC__ENET1_MDC,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_MDIO__ENET1_MDIO,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_RD0__ENET1_RGMII_RD0,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_RD1__ENET1_RGMII_RD1,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_RD2__ENET1_RGMII_RD2,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_RD3__ENET1_RGMII_RD3,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_RXC__ENET1_RGMII_RXC,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(3U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_RX_CTL__ENET1_RGMII_RX_CTL,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_TD0__ENET1_RGMII_TD0,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U));
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_TD1__ENET1_RGMII_TD1,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U));
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_TD2__ENET1_RGMII_TD2,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U));
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_TD3__ENET1_RGMII_TD3,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U));
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_TXC__ENET1_RGMII_TXC,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(3U));
    IOMUXC_SetPinConfig(IOMUXC_PAD_ENET2_TX_CTL__ENET1_RGMII_TX_CTL,
                        IOMUXC_PAD_DSE(63U) |
                        IOMUXC_PAD_FSEL1(2U));
}

void board_pins_setup(void)
{
    pin_mux_lpuart2();
    pin_mux_enet();
}
