/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "avb_hardware.h"
#include "fsl_enet_qos.h"
#include "fsl_iomuxc.h"

static void pin_mux_enet_qos(void)
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

static void BOARD_TpmClockSetup(void)
{
	const clock_root_config_t tpm2ClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_TPM2_ClockRoot_MuxAudioPll1Out, /* BOARD_TPM_REC_BASE_FREQ: 393216000HZ / 8 = 49,152MHz */
		.div = 8
	};
	const clock_root_config_t tpm4ClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_TPM4_ClockRoot_MuxOsc24M, /* 24MHz oscillator source */
		.div = 1
	};

	CLOCK_SetRootClock(kCLOCK_Root_Tpm2, &tpm2ClkCfg);
	CLOCK_SetRootClock(kCLOCK_Root_Tpm4, &tpm4ClkCfg);
}

static void enet_qos_clock_config(void)
{
	/* enetqosSysClk 250MHz */
	const clock_root_config_t enetqosSysClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
		.div = 4
	};

	/* enetqosPtpClk 100MHz */
	const clock_root_config_t enetqosPtpClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_ENETTSTMR2_ClockRoot_MuxSysPll1Pfd1Div2, // 400MHz
		.div = 4
	};

	/* enetqosClk 250MHz (For 125MHz TX_CLK ) */
	const clock_root_config_t enetqosClkCfg = {
		.clockOff = false,
		.mux = kCLOCK_ENET_ClockRoot_MuxSysPll1Pfd0Div2, // 500MHz
		.div = 2
	};

	CLOCK_DisableClock(kCLOCK_Enet_Qos);
	CLOCK_SetRootClock(kCLOCK_Root_WakeupAxi, &enetqosSysClkCfg);
	CLOCK_SetRootClock(kCLOCK_Root_EnetTimer2, &enetqosPtpClkCfg);
	CLOCK_SetRootClock(kCLOCK_Root_Enet, &enetqosClkCfg);
	CLOCK_EnableClock(kCLOCK_Enet_Qos);
}

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
	BLK_CTRL_WAKEUPMIX->GPR |= BLK_CTRL_WAKEUPMIX_GPR_MODE(miiMode);
}

void ENET_QOS_EnableClock(bool enable)
{
	 BLK_CTRL_WAKEUPMIX->GPR =
		(BLK_CTRL_WAKEUPMIX->GPR & (~BLK_CTRL_WAKEUPMIX_GPR_ENABLE_MASK)) | BLK_CTRL_WAKEUPMIX_GPR_ENABLE(enable);
}

void avb_hardware_init(void)
{
	pin_mux_enet_qos();

	BOARD_TpmClockSetup();

	enet_qos_clock_config();
}
