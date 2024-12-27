/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "fsl_common.h"
#include "board.h"
#include "mmu.h"
#include "uart.h"
#include "hal_pinctrl.h"

#include "FreeRTOS.h"
#include "sm_platform.h"
#include "os/irq.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static inline void pin_mux_lpuart3(void)
{
    HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO15__LPUART3_RX, 0U);
    HAL_PinctrlSetPinMux(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO14__LPUART3_TX, 0U);

    HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO15__LPUART3_RX,
    		HAL_PINCTRL_PLATFORM_IOMUXC_PAD_PD_MASK);
    HAL_PinctrlSetPinCfg(HAL_PINCTRL_PLATFORM_IOMUXC_PAD_GPIO_IO14__LPUART3_TX,
    		HAL_PINCTRL_PLATFORM_IOMUXC_PAD_DSE(15U));
}

static inline void clock_config_lpuart3(void)
{
    /* clang-format off */
    hal_clk_t hal_clk = {
        .clk_id = hal_clock_lpuart3,
        .pclk_id = hal_clock_osc24m,
        .div = 1,
        .enable_clk = true,
        .clk_round_opt = hal_clk_round_auto,
    };
    /* clang-format on */
    HAL_ClockSetRootClk(&hal_clk);
}

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    pin_mux_lpuart3();
    clock_config_lpuart3();
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
