/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/counter.h"
#include "os/irq.h"

#include "hlog.h"

#include "fsl_device_registers.h"
#include "fsl_gpt.h"

/* FIXME use fsl_clock to get the frequency */
#define SOURCE_CLOCK_FREQ_MHZ	24

/*
 * @returns the index number of the specified peripheral,
 *          matching arrays defined here as well as in mcux-sdk
 */
static uint8_t gpt_get_index(const GPT_Type *base)
{
	uint8_t index = 0;

	switch ((uintptr_t)base) {
		case (uintptr_t)GPT1: index = 1; break;
		case (uintptr_t)GPT2: index = 2; break;
		case (uintptr_t)GPT3: index = 3; break;
		case (uintptr_t)GPT4: index = 4; break;
		case (uintptr_t)GPT5: index = 5; break;
		case (uintptr_t)GPT6: index = 6; break;
		default: break;
	}

	return index;
}

static int set_alarm(os_counter_t *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm_cfg)
{
	struct os_counter_alarm_cfg *alarm = &dev->alarms[chan_id];

	os_assert(dev->initialized != true, "GPT device %p not initialized!", dev->base);

	/* save the alarm config */
	alarm->callback = alarm_cfg->callback;
	alarm->user_data = alarm_cfg->user_data;

	return (dev->initialized) ? 0 : -1;
}

static void reset_alarm(os_counter_t *dev, uint8_t chan_id)
{
	struct os_counter_alarm_cfg *alarm = &dev->alarms[chan_id];

	/* reset alarm config */
	alarm->callback = NULL;
}

static void gpt_irq_ack(const os_counter_t *dev, uint8_t chan_id)
{
	/* TODO: support multiple channels */
	if (chan_id != kGPT_OutputCompare_Channel1) {
		/* TODO: support multiple channels */
		log_err("Channel ID (%d) not supported!\n", chan_id);

	} else {
		GPT_DisableInterrupts((GPT_Type *)dev->base, kGPT_OutputCompare1InterruptEnable);
		GPT_ClearStatusFlags((GPT_Type *)dev->base, kGPT_OutputCompare1Flag);
	}
}

static void gpt_irq_handler(void *irq_dev)
{
	os_counter_t *dev = (os_counter_t *)irq_dev;
	const struct os_counter_alarm_cfg *alarm;
	/* TODO: support multiple channels */
	uint8_t chan_id = kGPT_OutputCompare_Channel1;
	uint32_t now;
	int ret;

	ret = os_counter_get_value(dev, &now);
	if (ret)
		assert(true);

	gpt_irq_ack(dev, chan_id);

	alarm = &dev->alarms[chan_id];

	if (alarm->callback)
		alarm->callback(dev, chan_id, now, alarm->user_data);

	reset_alarm(dev, chan_id);
}

static void counter_init(os_counter_t *dev)
{
	IRQn_Type irqn = dev->irqn;
	gpt_config_t gptConfig;
	int ret;

	GPT_GetDefaultConfig(&gptConfig);
	gptConfig.enableFreeRun = true;
	gptConfig.clockSource = kGPT_ClockSource_Periph;
	gptConfig.divider = 1;
	GPT_Init((GPT_Type *)(dev->base), &gptConfig);

	ret = os_irq_register(irqn, gpt_irq_handler, (void *)dev, OS_IRQ_PRIO_DEFAULT);
	os_assert(!ret, "Failed to register counter's IRQ! (%d)", ret);
	os_irq_enable(irqn);

	dev->initialized = true;

	log_debug("GPT dev %p irq %d initialized\n",
		dev->base, irqn);
}

int os_counter_start(os_counter_t *dev)
{
	uint8_t index = gpt_get_index(dev->base);

	os_assert(index != 0, "Unknown device %p", dev->base);

	if (!dev->initialized)
		counter_init(dev);

	GPT_StartTimer((GPT_Type *)dev->base);

	return 0;
}

int os_counter_stop(const os_counter_t *dev)
{
	GPT_StopTimer((GPT_Type *)dev->base);

	return 0;
}

int os_counter_get_value(const os_counter_t *dev, uint32_t *cnt)
{
	if (!cnt)
		return -1;

	*cnt = GPT_GetCurrentTimerCount((GPT_Type *)dev->base);

	return 0;
}

bool os_counter_is_counting_up(const os_counter_t *dev)
{
	return true;
}

uint32_t os_counter_us_to_ticks(const os_counter_t *dev, uint64_t period_us)
{
	return period_us * SOURCE_CLOCK_FREQ_MHZ;
}

uint64_t os_counter_ticks_to_ns(const os_counter_t *dev, uint32_t ticks)
{
	return (1000 * (uint64_t)ticks) / SOURCE_CLOCK_FREQ_MHZ;
}

uint32_t os_counter_get_top_value(const os_counter_t *dev)
{
	return UINT32_MAX;
}

uint8_t os_counter_get_num_of_channels(const os_counter_t *dev)
{
	return NB_CHANNELS;
}

/*
 * After expiration alarm can be set again, disabling is not needed.
 * When alarm expiration handler is called, channel is considered available and can be set again in that context.
 */
int os_counter_set_channel_alarm(os_counter_t *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm_cfg)
{
	int ret = 0;
	uint32_t current, next;

	if (!alarm_cfg) {
		log_err("Null pointer for channel ID (%d)\n", chan_id);

		ret = -1;
		goto exit;
	}

	if (chan_id != kGPT_OutputCompare_Channel1) {
		/* TODO: support multiple channels */
		log_err("Channel ID (%d) not supported!\n", chan_id);

		ret = -1;
		goto exit;
	}

	/* Keep a reference of the alarm config (for callback) */
	ret = set_alarm(dev, chan_id, alarm_cfg);
	if (ret) {
		log_err("Failed to set counter's alarm for device %p channel %d\n",
			       dev, chan_id);

		goto exit;
	}

	/* program compare register value with current counter + ticks to wait for */
	next = alarm_cfg->ticks;
	if (!(alarm_cfg->flags & OS_COUNTER_ALARM_CFG_ABSOLUTE)) {
		os_counter_get_value(dev, &current);
		next += current;
	}

	GPT_EnableInterrupts((GPT_Type *)dev->base, kGPT_OutputCompare1InterruptEnable);
	GPT_SetOutputCompareValue((GPT_Type *)dev->base, kGPT_OutputCompare_Channel1, next);

exit:
	return ret;
}

int os_counter_cancel_channel_alarm(os_counter_t *dev, uint8_t chan_id)
{
	int ret = 0;

	if (chan_id != kGPT_OutputCompare_Channel1) {
		/* TODO: support multiple channels */
		log_err("Channel ID (%d) not supported!\n", chan_id);

		ret = -1;
		goto exit;
	}

	GPT_DisableInterrupts((GPT_Type *)dev->base, kGPT_OutputCompare1InterruptEnable);
	GPT_ClearStatusFlags((GPT_Type *)dev->base, kGPT_OutputCompare1Flag);

	reset_alarm(dev, chan_id);

exit:
	return ret;
}

/* Init the generic counter structs. */
os_counter_t freertos_counter_instance_0 = {
       .base = GPT1,
       .irqn = GPT1_IRQn,
};

os_counter_t freertos_counter_instance_1 = {
       .base = GPT2,
       .irqn = GPT2_IRQn,
};
