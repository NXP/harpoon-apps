/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_iomuxc.h"
#include "pin_mux.h"

void pin_mux_enet_qos(void)
{
    /* ENET_QOS PHY RESET HPIO */
    IOMUXC_SetPinMux(IOMUXC_SAI2_RXC_GPIO4_IO22, 5U);
    IOMUXC_SetPinConfig(IOMUXC_SAI2_RXC_GPIO4_IO22,
            IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
            IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    /* MDC/MDIO */
    IOMUXC_SetPinMux(IOMUXC_ENET_MDC_ENET_QOS_MDC, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_MDC_ENET_QOS_MDC,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(1U));
    IOMUXC_SetPinMux(IOMUXC_ENET_MDIO_ENET_QOS_MDIO, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_MDIO_ENET_QOS_MDIO,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(1U));

    IOMUXC_SetPinMux(IOMUXC_ENET_RD0_ENET_QOS_RGMII_RD0, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_RD0_ENET_QOS_RGMII_RD0,
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_RD1_ENET_QOS_RGMII_RD1, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_RD1_ENET_QOS_RGMII_RD1,
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_RD2_ENET_QOS_RGMII_RD2, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_RD2_ENET_QOS_RGMII_RD2,
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_RD3_ENET_QOS_RGMII_RD3, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_RD3_ENET_QOS_RGMII_RD3,
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);

    IOMUXC_SetPinMux(IOMUXC_ENET_TD0_ENET_QOS_RGMII_TD0, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_TD0_ENET_QOS_RGMII_TD0,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_TD1_ENET_QOS_RGMII_TD1, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_TD1_ENET_QOS_RGMII_TD1,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_TD2_ENET_QOS_RGMII_TD2, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_TD2_ENET_QOS_RGMII_TD2,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_TD3_ENET_QOS_RGMII_TD3, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_TD3_ENET_QOS_RGMII_TD3,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);

    IOMUXC_SetPinMux(IOMUXC_ENET_RXC_CCM_ENET_QOS_CLOCK_GENERATE_RX_CLK, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_RXC_CCM_ENET_QOS_CLOCK_GENERATE_RX_CLK,
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_TXC_CCM_ENET_QOS_CLOCK_GENERATE_TX_CLK, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_TXC_CCM_ENET_QOS_CLOCK_GENERATE_TX_CLK,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);

    IOMUXC_SetPinMux(IOMUXC_ENET_RX_CTL_ENET_QOS_RGMII_RX_CTL, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_RX_CTL_ENET_QOS_RGMII_RX_CTL,
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_ENET_TX_CTL_ENET_QOS_RGMII_TX_CTL, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ENET_TX_CTL_ENET_QOS_RGMII_TX_CTL,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);
}

void pin_mux_flexcan1(void)
{
    IOMUXC_SetPinMux(IOMUXC_SPDIF_EXT_CLK_GPIO5_IO05, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SPDIF_EXT_CLK_GPIO5_IO05,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_SPDIF_RX_CAN1_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SPDIF_RX_CAN1_RX,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_SPDIF_TX_CAN1_TX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SPDIF_TX_CAN1_TX,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                        IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PE_MASK);
}

void BOARD_InitPins(void)
{
    pin_mux_enet_qos();
    pin_mux_flexcan1();
}
