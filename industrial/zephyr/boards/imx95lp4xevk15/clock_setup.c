/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/clock_control.h>

#define CAN_DEVICE DT_ALIAS(candev0)
const struct device *clock_dev = DEVICE_DT_GET(DT_CLOCKS_CTLR_BY_IDX(CAN_DEVICE, 0));
clock_control_subsys_t clock_subsys = (clock_control_subsys_t) DT_CLOCKS_CELL_BY_IDX(CAN_DEVICE, 0, name);

void clock_setup_flexcan(void)
{
	if (!device_is_ready(clock_dev)) {
		__ASSERT(false, "%s: clock_dev is not ready", __func__);
		return;
	}

	if (clock_control_on(clock_dev, clock_subsys) < 0) {
		__ASSERT(false, "%s: could not turn clock_dev on", __func__);
		return;
	}
}

int clock_get_flexcan_clock(uint32_t *rate)
{
	return clock_control_get_rate(clock_dev, clock_subsys, rate);
}

void BOARD_TpmClockSetup(void) {}
