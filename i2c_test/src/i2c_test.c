/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_i2c.h"
#include "i2c_test.h"
#include "os/stdio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* I2C */
#define I2C_INSTANCE I2C3
#define I2C_MASTER_CLK_FREQ                                                                 \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootI2c3)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootI2c3)) / 5) /* SYSTEM PLL1 DIV5 */

#ifdef CPU_MIMX8ML8DVNLZ_ca53
#define I2C_ADDR     0x50  /* Type-C0 on i.MX8MP EVK */
#else
#define I2C_ADDR     0x4a  /* PCM1863 on Hifiberry DAC+ ADC Pro */
#endif

#define I2C_BAUDRATE 100000U

uint8_t i2c_buf[256];

void i2c_tests(void)
{
    int i;
    i2c_master_config_t i2c_config;
    i2c_master_transfer_t i2c_transfer;

    /* Configure and enable I2C3 */
    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
    CLOCK_EnableClock(kCLOCK_I2c3);

    /* Initialise I2C */
    I2C_MasterGetDefaultConfig(&i2c_config);
    i2c_config.baudRate_Bps = I2C_BAUDRATE;
    I2C_MasterInit(I2C_INSTANCE, &i2c_config, I2C_MASTER_CLK_FREQ);

    memset(i2c_buf, 0x66, sizeof(i2c_buf));
    memset(&i2c_transfer, 0, sizeof(i2c_transfer));

    /* Read all I2C registers */
    for (i = 0; i < sizeof(i2c_buf); i++) {
        i2c_transfer.slaveAddress   = I2C_ADDR;
        i2c_transfer.direction      = kI2C_Read;
        i2c_transfer.subaddress     = i;
        i2c_transfer.subaddressSize = 1;
        i2c_transfer.data           = &i2c_buf[i];
        i2c_transfer.dataSize       = 1;
        i2c_transfer.flags          = kI2C_TransferDefaultFlag;

        I2C_MasterTransferBlocking(I2C_INSTANCE, &i2c_transfer);
    }

    /* Print out all I2C registers */
    os_printf("I2C registers dump for device at address 0x%x\r\n", I2C_ADDR);
    os_printf("---------------------------------------------");
    for (i = 0; i < sizeof(i2c_buf); i++) {
        if ((i % 16) == 0) os_printf("\r\n%02X   ", i);
        os_printf("%02X ", i2c_buf[i]);
    }
    os_printf("\r\n\r\n");
}
