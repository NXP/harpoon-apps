/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_iomuxc.h"

void BOARD_InitPins(void)
{
    IOMUXC_SetPinMux(IOMUXC_PAD_I2C1_SCL__LPI2C1_SCL, 1U);
    IOMUXC_SetPinMux(IOMUXC_PAD_I2C1_SDA__LPI2C1_SDA, 1U);

    IOMUXC_SetPinConfig(IOMUXC_PAD_I2C1_SCL__LPI2C1_SCL,
                        IOMUXC_PAD_DSE(15U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_OD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_I2C1_SDA__LPI2C1_SDA,
                        IOMUXC_PAD_DSE(15U) |
                        IOMUXC_PAD_FSEL1(2U) |
                        IOMUXC_PAD_OD_MASK);
}
