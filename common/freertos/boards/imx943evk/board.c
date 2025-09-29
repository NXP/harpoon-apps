/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_iomuxc.h"

#include "board.h"
#include "mmu.h"
#include "uart.h"

#include "FreeRTOS.h"
#include "sm_platform.h"
#include "os/irq.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static inline void pin_mux_lpuart12(void)
{
    IOMUXC_SetPinMux(IOMUXC_PAD_GPIO_IO27__LPUART12_RX, 0U);
    IOMUXC_SetPinMux(IOMUXC_PAD_GPIO_IO26__LPUART12_TX, 0U);

    IOMUXC_SetPinConfig(IOMUXC_PAD_GPIO_IO27__LPUART12_RX, IOMUXC_PAD_PD_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PAD_GPIO_IO26__LPUART12_TX, IOMUXC_PAD_DSE(15U));
}

static inline void clock_config_lpuart12(void)
{
    /* clang-format off */
    clk_t clk = {
        .clkId = kCLOCK_Lpuart12,
        .clkRoundOpt = SCMI_CLOCK_ROUND_AUTO,
        .rate = 24000000UL,
    };
    /* clang-format on */

    CLOCK_SetRate(&clk);
    CLOCK_EnableClock(clk.clkId);
}

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    pin_mux_lpuart12();
    clock_config_lpuart12();
    uart_init();
}

/* Initialize MMU, configure memory attributes for each region */
void BOARD_InitMemory(void)
{
    MMU_init();
}

void BOARD_InitPlatform(void)
{
    IRQn_Type const s_muIrqs[] = MU_IRQS;
    SM_Platform_Init();
    os_irq_register(s_muIrqs[SM_PLATFORM_MU_INST], SM_platform_MU_IRQHandler, NULL, portLOWEST_USABLE_INTERRUPT_PRIORITY - 1);
}

void BOARD_RdcInit(void)
{
}
