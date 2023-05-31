/*
 * Copyright 2021,2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "i2c_test.h"
#include "os/stdio.h"
#include "app_i2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifdef I2C_USE_IRQ
i2c_master_handle_t g_m_handle;
volatile bool g_MasterCompletionFlag = false;
#endif

uint8_t i2c_buf[256];

#ifdef I2C_USE_IRQ
/* i2c_callback
 * This function is called by I2C driver when a non-blocking operation is completed
 */
static void i2c_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
        g_MasterCompletionFlag = true;
    else
        os_printf("%s: received error %d\r\n", __FUNCTION__, status);
}

/* i2c_test_irq_handler
 * This function is called by I2C ISR handler
 */
void i2c_test_irq_handler(void)
{
    I2C_MasterTransferHandleIRQ(I2C_INSTANCE, &g_m_handle);
}
#endif

void i2c_test(void)
{
    int i;
    i2c_master_config_t i2c_config;
    i2c_master_transfer_t i2c_transfer;

#ifdef I2C_USE_IRQ
    os_printf("I2C in interrupt mode\r\n");
#else
    os_printf("I2C in polling mode\r\n");
#endif

    /* Configure and enable I2C3 */
    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
    CLOCK_EnableClock(kCLOCK_I2c3);

    /* Initialise I2C */
    I2C_MasterGetDefaultConfig(&i2c_config);
    i2c_config.baudRate_Bps = I2C_BAUDRATE;
    I2C_MasterInit(I2C_INSTANCE, &i2c_config, I2C_MASTER_CLK_FREQ);

#ifdef I2C_USE_IRQ
    memset(&g_m_handle, 0, sizeof(g_m_handle));
#endif
    memset(i2c_buf, 0x66, sizeof(i2c_buf));
    memset(&i2c_transfer, 0, sizeof(i2c_transfer));

#ifdef I2C_USE_IRQ
    I2C_MasterTransferCreateHandle(I2C_INSTANCE, &g_m_handle, i2c_callback, NULL);
#endif

    /* Read all I2C registers */
    for (i = 0; i < sizeof(i2c_buf); i++) {
        i2c_transfer.slaveAddress   = I2C_ADDR;
        i2c_transfer.direction      = kI2C_Read;
        i2c_transfer.subaddress     = i;
        i2c_transfer.subaddressSize = 1;
        i2c_transfer.data           = &i2c_buf[i];
        i2c_transfer.dataSize       = 1;
        i2c_transfer.flags          = kI2C_TransferDefaultFlag;

#ifdef I2C_USE_IRQ
        I2C_MasterTransferNonBlocking(I2C_INSTANCE, &g_m_handle, &i2c_transfer);
        /*  Wait for transfer completed. */
        while (!g_MasterCompletionFlag) /* nothing */;
        g_MasterCompletionFlag = false;
#else
        I2C_MasterTransferBlocking(I2C_INSTANCE, &i2c_transfer);
#endif
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
