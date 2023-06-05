/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_I2C_H_
#define _APP_I2C_H_

#include "fsl_i2c.h"

#define I2C_ADDR     0x50  /* Type-C0 on i.MX8MP EVK */
#define I2C_INSTANCE I2C3
#define I2C_BAUDRATE 100000U
#define I2C_IRQn     I2C3_IRQn

#define I2C_MASTER_CLK_FREQ                                                                 \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootI2c3)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootI2c3)) / 5) /* SYSTEM PLL1 DIV5 */

void i2c_driver_init(void);
uint8_t i2c_driver_read_reg(uint8_t i2c_addr, uint32_t sub_addr);
void i2c_driver_irq_handler(void);

#endif /* _APP_I2C_H_ */
