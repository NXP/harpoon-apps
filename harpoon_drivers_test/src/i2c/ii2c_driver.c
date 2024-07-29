/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_i2c.h"
#include "i2c_test.h"
#include "app_i2c.h"

#include "rtos_abstraction_layer.h"

#ifdef I2C_USE_IRQ
i2c_master_handle_t g_m_handle;
volatile bool g_MasterCompletionFlag = false;
#endif

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
        rtos_printf("%s: received error %d\r\n", __FUNCTION__, status);
}

void i2c_driver_irq_handler(void)
{
    I2C_MasterTransferHandleIRQ(I2C_INSTANCE, &g_m_handle);
}
#endif

void i2c_driver_init(void)
{
    i2c_master_config_t i2c_config;

    I2C_MasterGetDefaultConfig(&i2c_config);
    i2c_config.baudRate_Bps = I2C_BAUDRATE;
    I2C_MasterInit(I2C_INSTANCE, &i2c_config, I2C_MASTER_CLK_FREQ);

#ifdef I2C_USE_IRQ
    memset(&g_m_handle, 0, sizeof(g_m_handle));
    I2C_MasterTransferCreateHandle(I2C_INSTANCE, &g_m_handle, i2c_callback, NULL);
#endif
}

/* Read I2C register */
uint8_t i2c_driver_read_reg(uint8_t i2c_addr, uint32_t sub_addr)
{
    uint8_t buf;
    i2c_master_transfer_t i2c_transfer;

    memset(&i2c_transfer, 0, sizeof(i2c_transfer));

    i2c_transfer.slaveAddress   = i2c_addr;
    i2c_transfer.direction      = kI2C_Read;
    i2c_transfer.subaddress     = sub_addr;
    i2c_transfer.subaddressSize = 1;
    i2c_transfer.data           = &buf;
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

    return buf;
}
