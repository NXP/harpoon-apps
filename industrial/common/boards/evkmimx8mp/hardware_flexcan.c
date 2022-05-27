/*
* Copyright 2022 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "fsl_gpio.h"

void hardware_flexcan_init(void)
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};

    /* GPIO5_IO05 is used to control CAN1_STBY which is enabled active high */
    GPIO_PinInit(GPIO5, 5U, &gpio_config);
}
