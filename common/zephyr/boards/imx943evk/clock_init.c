/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/firmware/scmi/clk.h>
#include <zephyr/dt-bindings/clock/imx943_clock.h>

#define FREQ_24M_HZ 24000000 /* 24 MHz */

static int lpuart_clk_init(void)
{
#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpuart12), okay)
	const struct device *clk_dev = DEVICE_DT_GET(DT_NODELABEL(scmi_clk));
	struct scmi_protocol *proto = clk_dev->data;
	struct scmi_clock_rate_config clk_cfg = {0};
	uint32_t clk_id;
	struct scmi_clock_config cfg;
	int ret;

	clk_id = IMX943_CLK_LPUART12;

	ret = scmi_clock_parent_set(proto, clk_id, IMX943_CLK_24M);
	if (ret) {
		return ret;
	}

	cfg.attributes = SCMI_CLK_CONFIG_ENABLE_DISABLE(true);
	cfg.clk_id = clk_id;
	ret = scmi_clock_config_set(proto, &cfg);
	if (ret) {
		return ret;
	}

	clk_cfg.flags = SCMI_CLK_RATE_SET_FLAGS_ROUNDS_AUTO;
	clk_cfg.clk_id = clk_id;
	clk_cfg.rate[0] = FREQ_24M_HZ; /* 24 MHz*/
	clk_cfg.rate[1] = 0;

	ret = scmi_clock_rate_set(proto, &clk_cfg);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

SYS_INIT(lpuart_clk_init, PRE_KERNEL_2, 0);
