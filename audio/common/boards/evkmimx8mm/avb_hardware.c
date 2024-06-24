/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "avb_hardware.h"

#include "fsl_clock.h"
#include "fsl_enet.h"
#include "fsl_iomuxc.h"

static void pin_mux_enet(void)
{
	IOMUXC_SetPinMux(IOMUXC_ENET_MDC_ENET1_MDC, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_MDC_ENET1_MDC,
			IOMUXC_SW_PAD_CTL_PAD_DSE(3U));
	IOMUXC_SetPinMux(IOMUXC_ENET_MDIO_ENET1_MDIO, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_MDIO_ENET1_MDIO,
			IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

	IOMUXC_SetPinMux(IOMUXC_ENET_RD0_ENET1_RGMII_RD0, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_RD0_ENET1_RGMII_RD0,
			IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));
	IOMUXC_SetPinMux(IOMUXC_ENET_RD1_ENET1_RGMII_RD1, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_RD1_ENET1_RGMII_RD1,
			IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));
	IOMUXC_SetPinMux(IOMUXC_ENET_RD2_ENET1_RGMII_RD2, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_RD2_ENET1_RGMII_RD2,
			IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));
	IOMUXC_SetPinMux(IOMUXC_ENET_RD3_ENET1_RGMII_RD3, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_RD3_ENET1_RGMII_RD3,
			IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));

	IOMUXC_SetPinMux(IOMUXC_ENET_RX_CTL_ENET1_RGMII_RX_CTL, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_RX_CTL_ENET1_RGMII_RX_CTL,
			IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));

	IOMUXC_SetPinMux(IOMUXC_ENET_RXC_ENET1_RGMII_RXC, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_RXC_ENET1_RGMII_RXC,
			IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
			IOMUXC_SW_PAD_CTL_PAD_HYS(1U));

	IOMUXC_SetPinMux(IOMUXC_ENET_TD0_ENET1_RGMII_TD0, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_TD0_ENET1_RGMII_TD0,
			IOMUXC_SW_PAD_CTL_PAD_DSE(7U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(3U));
	IOMUXC_SetPinMux(IOMUXC_ENET_TD1_ENET1_RGMII_TD1, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_TD1_ENET1_RGMII_TD1,
			IOMUXC_SW_PAD_CTL_PAD_DSE(7U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(3U));
	IOMUXC_SetPinMux(IOMUXC_ENET_TD2_ENET1_RGMII_TD2, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_TD2_ENET1_RGMII_TD2,
			IOMUXC_SW_PAD_CTL_PAD_DSE(7U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(3U));
	IOMUXC_SetPinMux(IOMUXC_ENET_TD3_ENET1_RGMII_TD3, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_TD3_ENET1_RGMII_TD3,
			IOMUXC_SW_PAD_CTL_PAD_DSE(7U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(3U));

	IOMUXC_SetPinMux(IOMUXC_ENET_TX_CTL_ENET1_RGMII_TX_CTL, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_TX_CTL_ENET1_RGMII_TX_CTL,
			IOMUXC_SW_PAD_CTL_PAD_DSE(7U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(3U));

	IOMUXC_SetPinMux(IOMUXC_ENET_TXC_ENET1_RGMII_TXC, 0U);
	IOMUXC_SetPinConfig(IOMUXC_ENET_TXC_ENET1_RGMII_TXC,
			IOMUXC_SW_PAD_CTL_PAD_DSE(7U) |
			IOMUXC_SW_PAD_CTL_PAD_FSEL(3U));
}

static void enet_clock_config(void)
{
	CLOCK_DisableClock(kCLOCK_Enet1);

	CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);
	CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3);

	CLOCK_SetRootMux(kCLOCK_RootEnetTimer, kCLOCK_EnetTimerRootmuxSysPll2Div10);
	CLOCK_SetRootDivider(kCLOCK_RootEnetTimer, 1U, 1U);

	CLOCK_EnableClock(kCLOCK_Enet1);
}

static void gpt_clock_config(void)
{
	CLOCK_DisableClock(kCLOCK_Gpt1);
	CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxOsc24M);
	CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 1U);
	CLOCK_EnableClock(kCLOCK_Gpt1);

	CLOCK_DisableClock(kCLOCK_Gpt2);
	CLOCK_SetRootMux(kCLOCK_RootGpt2, kCLOCK_GptRootmuxOsc24M);
	CLOCK_SetRootDivider(kCLOCK_RootGpt2, 1U, 1U);
	CLOCK_EnableClock(kCLOCK_Gpt2);
}

void avb_hardware_init(void)
{
	pin_mux_enet();

	enet_clock_config();
	gpt_clock_config();

	/* TODO: PHY reset? */
}
