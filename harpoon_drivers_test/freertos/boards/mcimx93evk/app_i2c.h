/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_I2C_H_
#define _APP_I2C_H_

#include "fsl_lpi2c.h"

#define I2C_ADDR		0x4a
#define I2C_INSTANCE		LPI2C1
#define I2C_IRQn		LPI2C1_IRQn
#define I2C_BAUDRATE		100000U
#define I2C_MASTER_CLK_FREQ	(CLOCK_GetIpFreq(kCLOCK_Root_Lpi2c1))

void i2c_driver_init(void);
uint8_t i2c_driver_read_reg(uint8_t i2c_addr, uint32_t sub_addr);
void i2c_driver_irq_handler(void);

#endif /* _APP_I2C_H_ */
