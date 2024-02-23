/*
* Copyright 2022-2023 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "hardware_ethernet.h"
#include "fsl_gpio.h"

phy_rtl8211f_resource_t phy_resource;

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL(miiMode); /* Set this bit to enable ENET_QOS clock generation. */
}

void ENET_QOS_EnableClock(bool enable)
{
	IOMUXC_GPR->GPR1 = (IOMUXC_GPR->GPR1 & (~IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN_MASK)) |
		IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN(enable);
}

void hardware_ethernet_init(void)
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    GPIO_PinInit(GPIO4, 22, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
    * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO4, 22, 0);
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    GPIO_WritePinOutput(GPIO4, 22, 1);
    SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    /* Set this bit to enable ENET_QOS RGMII TX clock output on TX_CLK pad. */
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_IOMUXC_GPR_ENET_QOS_RGMII_EN(1);
}
