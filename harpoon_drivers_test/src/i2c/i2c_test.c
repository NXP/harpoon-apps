/*
 * Copyright 2021, 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "i2c_test.h"
#include "app_i2c.h"

#include "rtos_abstraction_layer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

uint8_t i2c_buf[256];

#ifdef I2C_USE_IRQ
/* i2c_test_irq_handler
 * This function is called by I2C ISR handler
 */
void i2c_test_irq_handler(void)
{
    i2c_driver_irq_handler();
}
#endif

void i2c_test(void)
{
    int i;

#ifdef I2C_USE_IRQ
    rtos_printf("I2C in interrupt mode\r\n");
#else
    rtos_printf("I2C in polling mode\r\n");
#endif

    /* Initialise I2C */
    i2c_driver_init();

    memset(i2c_buf, 0x66, sizeof(i2c_buf));

    /* Read I2C all registers */
    for(i = 0; i < sizeof(i2c_buf); i++)
        i2c_buf[i] = i2c_driver_read_reg(I2C_ADDR, i);

    /* Print out all I2C registers */
    rtos_printf("I2C registers dump for device at address 0x%x\r\n", I2C_ADDR);
    rtos_printf("---------------------------------------------");
    for (i = 0; i < sizeof(i2c_buf); i++) {
        if ((i % 16) == 0) rtos_printf("\r\n%02X   ", i);
        rtos_printf("%02X ", i2c_buf[i]);
    }
    rtos_printf("\r\n\r\n");
}
