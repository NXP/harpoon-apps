/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_iomuxc.h"
#include "pin_mux.h"

void BOARD_InitPins(void)
{
	/* iomux for I2C3 */
	IOMUXC_SetPinMux(IOMUXC_I2C3_SCL_I2C3_SCL, 1U);
	IOMUXC_SetPinConfig(IOMUXC_I2C3_SCL_I2C3_SCL,
			IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
			IOMUXC_SW_PAD_CTL_PAD_PUE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U) |
			IOMUXC_SW_PAD_CTL_PAD_PE(1U));

	IOMUXC_SetPinMux(IOMUXC_I2C3_SDA_I2C3_SDA, 1U);
	IOMUXC_SetPinConfig(IOMUXC_I2C3_SDA_I2C3_SDA,
			IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
			IOMUXC_SW_PAD_CTL_PAD_PUE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U) |
			IOMUXC_SW_PAD_CTL_PAD_PE(1U));

	/* iomux for SAI5 */
	IOMUXC_SetPinMux(IOMUXC_SAI5_RXD1_SAI5_TX_SYNC, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SAI5_RXD1_SAI5_TX_SYNC,
			IOMUXC_SW_PAD_CTL_PAD_DSE(6U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_PUE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));

	IOMUXC_SetPinMux(IOMUXC_SAI5_RXD2_SAI5_TX_BCLK, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SAI5_RXD2_SAI5_TX_BCLK,
			IOMUXC_SW_PAD_CTL_PAD_DSE(6U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_PUE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));

	IOMUXC_SetPinMux(IOMUXC_SAI5_RXD3_SAI5_TX_DATA0, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SAI5_RXD3_SAI5_TX_DATA0,
			IOMUXC_SW_PAD_CTL_PAD_DSE(6U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_PUE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));

	IOMUXC_SetPinMux(IOMUXC_SAI5_RXD0_SAI5_RX_DATA0, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SAI5_RXD0_SAI5_RX_DATA0,
			IOMUXC_SW_PAD_CTL_PAD_DSE(6U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_PUE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));
}
