/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/pinctrl.h>

#define CAN_DEVICE DT_ALIAS(candev0)
PINCTRL_DT_DEFINE(CAN_DEVICE);
PINCTRL_DT_DEV_CONFIG_DECLARE(CAN_DEVICE);
static struct pinctrl_dev_config *pcfg0 = PINCTRL_DT_DEV_CONFIG_GET(CAN_DEVICE);

#define ETH_DEVICE DT_ALIAS(ethernet0)
PINCTRL_DT_DEFINE(ETH_DEVICE);
PINCTRL_DT_DEV_CONFIG_DECLARE(ETH_DEVICE);
static struct pinctrl_dev_config *pcfg1 = PINCTRL_DT_DEV_CONFIG_GET(ETH_DEVICE);

/*
 * - Used pin:
 *   - {pin_num: W21, peripheral: CAN2, signal: can_rx, pin_signal: GPIO_IO27, HYS: DISABLED}
 *   - {pin_num: V21, peripheral: CAN2, signal: can_tx, pin_signal: GPIO_IO25, HYS: DISABLED}
 */
void pin_mux_flexcan(void)
{
	if (pinctrl_apply_state(pcfg0, PINCTRL_STATE_DEFAULT) < 0)
		__ASSERT(false, "%s: could apply pinmux", __func__);
}

void pin_mux_enetc(void)
{
	if (pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT) < 0)
		__ASSERT(false, "%s: could apply pinmux", __func__);
}
