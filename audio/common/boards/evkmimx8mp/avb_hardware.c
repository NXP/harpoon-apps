/*
* Copyright 2022 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "avb_hardware.h"
#include "fsl_enet_qos.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

static void pin_mux_enet_qos(void)
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

static void enet_qos_clock_config(void)
{
	CLOCK_DisableClock(kCLOCK_Enet_Qos);

	CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll1Div3);
	CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);

	CLOCK_SetRootMux(kCLOCK_RootEnetQos, kCLOCK_EnetQosRootmuxSysPll2Div8);
	CLOCK_SetRootDivider(kCLOCK_RootEnetQos, 1U, 1U);

	CLOCK_SetRootMux(kCLOCK_RootEnetQosTimer, kCLOCK_EnetQosTimerRootmuxSysPll2Div10);
	CLOCK_SetRootDivider(kCLOCK_RootEnetQosTimer, 1U, 1U);

	CLOCK_EnableClock(kCLOCK_Enet_Qos);
}

static void gpt_clock_config(void)
{
	CLOCK_DisableClock(kCLOCK_Gpt1);
	CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxAudioPll1);
	CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 32U); // 393216000HZ / 32 = 12,288MHz
	CLOCK_EnableClock(kCLOCK_Gpt1);

	CLOCK_DisableClock(kCLOCK_Gpt2);
	CLOCK_SetRootMux(kCLOCK_RootGpt2, kCLOCK_GptRootmuxOsc24M);
	CLOCK_SetRootDivider(kCLOCK_RootGpt2, 1U, 1U);
	CLOCK_EnableClock(kCLOCK_Gpt2);
}

void ENET_QOS_EnableClock(bool enable)
{
	IOMUXC_GPR->GPR1 = (IOMUXC_GPR->GPR1 & (~IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN_MASK)) |
		IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN(enable);
}

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
	IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL(miiMode); /* Set this bit to enable ENET_QOS clock generation. */
}

void avb_hardware_init(void)
{
	gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

	pin_mux_enet_qos();

	enet_qos_clock_config();
	gpt_clock_config();

	GPIO_PinInit(GPIO4, 22, &gpio_config);
	/* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
	* wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
	GPIO_WritePinOutput(GPIO4, 22, 0);
	SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
	GPIO_WritePinOutput(GPIO4, 22, 1);
	SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

	/* Set this bit to enable ENET_QOS RGMII TX clock output on TX_CLK pad. */
	IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_IOMUXC_GPR_ENET_QOS_RGMII_EN(1);
	/* Select GPT1 capture channel 2 input: ENET_QOS_TIMER1_EVENT */
	IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GPR_GPT1_CAPIN2_SEL(1);
}
