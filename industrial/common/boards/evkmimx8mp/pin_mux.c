/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_iomuxc.h"
#include "pin_mux.h"

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
	pin_mux_flexcan1();
}
