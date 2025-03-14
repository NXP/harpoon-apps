/*
 * Copyright 2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "app_board.h"
#include "clock_init.h"

static void BOARD_TpmClockSetup(void)
{
	const clock_root_config_t tpmClkCfg = {
		.clockOff = false,
		.mux = 0, /* 24MHz oscillator source */
		.div = 1
	};

	CLOCK_SetRootClock(kCLOCK_Root_Tpm2, &tpmClkCfg);
	CLOCK_SetRootClock(kCLOCK_Root_Tpm4, &tpmClkCfg);
}

static void clock_setup_enet_qos(void)
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

static void clock_setup_flexcan(void)
{
	/* clang-format off */
	const clock_root_config_t flexcanClkCfg = {
		.clockOff = false,
		.mux = 2, /* SYS_PLL_PFD1_DIV2 source clock */
		.div = 10
	};

	CLOCK_DisableClock(FLEXCAN_CLOCK_GATE);
	CLOCK_SetRootClock(FLEXCAN_CLOCK_ROOT, &flexcanClkCfg);
	CLOCK_EnableClock(FLEXCAN_CLOCK_GATE);
}

void board_clock_setup(void)
{
	/* board clock initialization must be run firstly */
	BOARD_ClockSourceFreqInit();
	BOARD_LpuartClockSetup();
	BOARD_TpmClockSetup();
	clock_setup_flexcan();
	clock_setup_enet_qos();
}

int board_clock_get_flexcan_rate(uint32_t *rate)
{
	if (rate) {
		*rate = CLOCK_GetIpFreq(FLEXCAN_CLOCK_ROOT);
		return 0;
	} else {
		return -1;
	}
}
