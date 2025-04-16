/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/firmware/scmi/power.h>
#include <zephyr/dt-bindings/power/imx95_power.h>

/* SCMI power domain states */
#define POWER_DOMAIN_STATE_ON  0x00000000
#define POWER_DOMAIN_STATE_OFF 0x40000000

void netcmix_init(void)
{
	uint32_t power_state;

	if(scmi_power_state_get(IMX95_PD_NETC, &power_state)) {
		__ASSERT(false, "%s: scmi_power_state_get failed", __func__);
		return;
	}

	if (power_state != POWER_DOMAIN_STATE_ON) {
		__ASSERT(false, "%s: IMX95_PD_NETC is off", __func__);
		return;
	}
}
