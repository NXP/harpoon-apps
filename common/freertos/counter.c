/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/counter.h"
#include "os/stdio.h"

/* FIXME use fsl_clock to get the frequency */
#define SOURCE_CLOCK_FREQ_MHZ	24

void os_counter_init(const void *dev)
{
    gpt_config_t gptConfig;

    GPT_GetDefaultConfig(&gptConfig);
    gptConfig.enableFreeRun = true;
    gptConfig.clockSource = kGPT_ClockSource_Periph;
    gptConfig.divider = 1;
    GPT_Init((GPT_Type *)(dev), &gptConfig);
}

int os_counter_start(const void *dev)
{
	GPT_StartTimer((GPT_Type *)dev);

	return 0;
}

int os_counter_stop(const void *dev)
{
	GPT_StopTimer((GPT_Type *)dev);

	return 0;
}

int os_counter_get_value(const void *dev, uint32_t *cnt)
{
	if (!cnt)
		return -1;

	*cnt = GPT_GetCurrentTimerCount((GPT_Type *)dev);

	return 0;
}

bool os_counter_is_counting_up(const void *dev)
{
	return true;
}

uint32_t os_counter_us_to_ticks(const void *dev, uint64_t period_us)
{
	return period_us * SOURCE_CLOCK_FREQ_MHZ;
}

uint64_t os_counter_ticks_to_ns(const void *dev, uint32_t ticks)
{
	return (1000 * ticks) / SOURCE_CLOCK_FREQ_MHZ;
}

uint32_t os_counter_get_top_value(const void *dev)
{
	return UINT32_MAX;
}

uint8_t os_counter_get_num_of_channels(const void *dev)
{
	/* TODO: support multiple channels */
	return 1;
}

int os_counter_set_channel_alarm(const void *dev, uint8_t chan_id,
          const struct os_counter_alarm_cfg *alarm_cfg)
{
	int ret = 0;
	uint32_t current, next;

	if (!alarm_cfg) {
		os_printf("Error: Null pointer for channel ID (%d)\n\r", chan_id);

		ret = -1;
		goto exit;
	}

	if (chan_id != kGPT_OutputCompare_Channel1) {
		/* TODO: support multiple channels */
		os_printf("Error: Channel ID (%d) not supported!\n\r", chan_id);

		ret = -1;
		goto exit;
	}

	/* program compare register value with current counter + ticks to wait for */
	next = alarm_cfg->ticks;
	if (!(alarm_cfg->flags & OS_COUNTER_ALARM_CFG_ABSOLUTE)) {
		os_counter_get_value(dev, &current);
		next += current;
	}

	GPT_EnableInterrupts((GPT_Type *)dev, kGPT_OutputCompare1InterruptEnable);
	GPT_SetOutputCompareValue((GPT_Type *)dev, kGPT_OutputCompare_Channel1, next);

exit:
	return ret;
}

int os_counter_reset_channel_alarm(const void *dev, uint8_t chan_id)
{
	int ret = 0;

	if (chan_id != kGPT_OutputCompare_Channel1) {
		/* TODO: support multiple channels */
		os_printf("Error: Channel ID (%d) not supported!\n\r", chan_id);

		ret = -1;
		goto exit;
	}

	GPT_DisableInterrupts((GPT_Type *)dev, kGPT_OutputCompare1InterruptEnable);
	GPT_ClearStatusFlags((GPT_Type *)dev, kGPT_OutputCompare1Flag);

exit:
	return ret;
}
