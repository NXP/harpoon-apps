/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_setup.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/clock_control.h>
#include "MIMX9596_ca55.h"

#define CAN_DEVICE DT_ALIAS(candev0)
const struct device *can_clk_dev = DEVICE_DT_GET(DT_CLOCKS_CTLR_BY_IDX(CAN_DEVICE, 0));
clock_control_subsys_t can_clk_subsys = (clock_control_subsys_t) DT_CLOCKS_CELL_BY_IDX(CAN_DEVICE, 0, name);

#define NETC_DEVICE DT_ALIAS(netc0)
const struct device *netc_clk_dev = DEVICE_DT_GET(DT_CLOCKS_CTLR_BY_IDX(NETC_DEVICE, 0));
clock_control_subsys_t netc_clk_subsys = (clock_control_subsys_t) DT_CLOCKS_CELL_BY_IDX(NETC_DEVICE, 0, name);

#define NETC_TIMER_DEVICE DT_ALIAS(netctimer0)
const struct device *netc_tmr_clk_dev = DEVICE_DT_GET(DT_CLOCKS_CTLR_BY_IDX(NETC_TIMER_DEVICE, 0));
clock_control_subsys_t netc_tmr_clk_subsys = (clock_control_subsys_t) DT_CLOCKS_CELL_BY_IDX(NETC_TIMER_DEVICE, 0, name);

#define COUNTER_0_DEVICE DT_ALIAS(counter0)
const struct device *tpm2_clk_dev = DEVICE_DT_GET(DT_CLOCKS_CTLR_BY_IDX(COUNTER_0_DEVICE, 0));
clock_control_subsys_t tpm2_clk_subsys = (clock_control_subsys_t) DT_CLOCKS_CELL_BY_IDX(COUNTER_0_DEVICE, 0, name);

#define COUNTER_1_DEVICE DT_ALIAS(counter1)
const struct device *tpm4_clk_dev = DEVICE_DT_GET(DT_CLOCKS_CTLR_BY_IDX(COUNTER_1_DEVICE, 0));
clock_control_subsys_t tpm4_clk_subsys = (clock_control_subsys_t) DT_CLOCKS_CELL_BY_IDX(COUNTER_1_DEVICE, 0, name);

void clock_setup_flexcan(void)
{
	if (!device_is_ready(can_clk_dev)) {
		__ASSERT(false, "%s: clock_dev is not ready", __func__);
		return;
	}

	if (clock_control_on(can_clk_dev, can_clk_subsys) < 0) {
		__ASSERT(false, "%s: could not turn clock_dev on", __func__);
		return;
	}
}

int clock_get_flexcan_clock(uint32_t *rate)
{
	return clock_control_get_rate(can_clk_dev, can_clk_subsys, rate);
}

void BOARD_TpmClockSetup(void) {
	if (!device_is_ready(tpm4_clk_dev)) {
		__ASSERT(false, "%s: clock_dev is not ready", __func__);
		return;
	}

	if (clock_control_on(tpm4_clk_dev, tpm4_clk_subsys) < 0) {
		__ASSERT(false, "%s: could not turn clock_dev on", __func__);
		return;
	}
}

void clock_setup_enetc(void)
{
	uint32_t rate;

	/* Host OS is the owner of the NETC reference and system clocks. So, we need
	* to only check if the clocks are on here. But, since clock_control_get_status()
	* is not available for SCMI clock control driver, just call the get rate and check that
	* it has the right frequency.
	*/
	if (!device_is_ready(netc_clk_dev)) {
		__ASSERT(false, "%s: clock_dev is not ready", __func__);
		return;
	}

	if (clock_control_get_rate(netc_clk_dev, netc_clk_subsys, &rate))
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	__ASSERT(rate == 250000000, "%s: enetc_clk rate(%d Hz) sould be at 250000000Hz", __func__, rate);

	if (!device_is_ready(netc_tmr_clk_dev)) {
		__ASSERT(false, "%s: clock_dev is not ready", __func__);
		return;
	}

	if (clock_control_get_rate(netc_tmr_clk_dev, netc_tmr_clk_subsys, &rate))
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	__ASSERT(rate == 666666666, "%s: netc_tmr_clk rate(%d Hz) sould be at 666666666Hz", __func__, rate);
}

uint32_t clock_get_mdio_clock(void)
{
	uint32_t rate;

	if (clock_control_get_rate(netc_clk_dev, netc_clk_subsys, &rate))
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	if (!rate)
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	return rate;
}

uint32_t clock_get_netc_timer_clock(void)
{
	uint32_t rate;

	if (clock_control_get_rate(netc_tmr_clk_dev, netc_tmr_clk_subsys, &rate))
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	if (!rate)
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	return rate;
}

uint32_t clock_get_tpm_clock(void *base)
{
	clock_control_subsys_t tpm_sub_sys;
	const struct device *tpm_dev;
	uint32_t rate = 0;

	if (base == TPM2) {
		tpm_dev = tpm2_clk_dev;
		tpm_sub_sys = tpm2_clk_subsys;
	} else if (base == TPM4) {
		tpm_dev = tpm4_clk_dev;
		tpm_sub_sys = tpm4_clk_subsys;
	} else {
		return 0;
	}

	if (clock_control_get_rate(tpm_dev, tpm_sub_sys, &rate))
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	if (!rate)
		__ASSERT(false, "%s: could not get rate of clock_dev", __func__);

	return rate;
}
