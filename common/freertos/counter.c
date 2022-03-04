/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/counter.h"

#include "irq.h"

#include "fsl_device_registers.h"
#include "fsl_gpt.h"

/* FIXME use fsl_clock to get the frequency */
#define SOURCE_CLOCK_FREQ_MHZ	24

/*
 * Support all GPT counters.
 * Note that each counter supports 3 Output Compare channels
 */
#define NB_COUNTERS		(sizeof(gpt_devices) / sizeof(GPT_Type *) - 1)
/* TODO: support multiple channels */
#define NB_CHANNELS		1

__WEAK void log_err(const char *format, ...) {};
__WEAK void log_debug(const char *format, ...) {};

struct counter_alarm {
	/* TODO: Add a mutex lock to protect both entries below */
	const void *dev; /* NULL if HW not initialized */
	const struct os_counter_alarm_cfg *alarms[NB_CHANNELS];
};

/* only used to compute NB_COUNTERS */
static GPT_Type *gpt_devices[] = GPT_BASE_PTRS;

/* index 0 is unused to match MCUXpresso array definitions */
static struct counter_alarm counters[NB_COUNTERS + 1];

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

/*
 * @returns the interrupt vector for the specified GPT peripheral
 */
static IRQn_Type gpt_get_irqn(const GPT_Type *base)
{
	const IRQn_Type irqs[] = GPT_IRQS;
	uint8_t index = gpt_get_index(base);

	return irqs[index];
}

static int set_alarm(const void *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm)
{
	uint8_t i = gpt_get_index(dev);
	struct counter_alarm *counter = &counters[i];

	os_assert(counter->dev != NULL, "Device %p not initialized!", dev);

	/* save the reference of the alarm config */
	counter->alarms[chan_id] = alarm;

	return (counter->dev) ? 0 : -1;
}

static void reset_alarm(const void *dev, uint8_t chan_id)
{
	uint8_t i = gpt_get_index(dev);
	struct counter_alarm *counter = &counters[i];

	/* reset alarm config */
	counter->alarms[chan_id] = NULL;
}

static const struct os_counter_alarm_cfg *get_alarm(const void *dev, uint8_t chan_id)
{
	uint8_t i = gpt_get_index(dev);
	struct counter_alarm *counter = &counters[i];
	const struct os_counter_alarm_cfg *alarm = NULL;

	if (counter && counter->dev)
		alarm = counter->alarms[chan_id];

	return alarm;
}

static void gpt_irq_ack(const void *dev, uint8_t chan_id)
{
	/* TODO: support multiple channels */
	if (chan_id != kGPT_OutputCompare_Channel1) {
		/* TODO: support multiple channels */
		log_err("Channel ID (%d) not supported!\n", chan_id);

	} else {
		GPT_DisableInterrupts((GPT_Type *)dev, kGPT_OutputCompare1InterruptEnable);
		GPT_ClearStatusFlags((GPT_Type *)dev, kGPT_OutputCompare1Flag);
	}
}

static void gpt_irq_handler(void *dev)
{
	const struct os_counter_alarm_cfg *alarm;
	uint32_t now;
	int ret;
	/* TODO: support multiple channels */
	uint8_t chan_id = kGPT_OutputCompare_Channel1;

	ret = os_counter_get_value(dev, &now);
	if (ret)
		assert(true);

	gpt_irq_ack(dev, chan_id);

	/* retrieve callback for this counter/channel and call it */
	alarm = get_alarm(dev, chan_id);

	if (alarm && alarm->callback)
		alarm->callback(dev, chan_id, now, alarm->user_data);

	reset_alarm(dev, chan_id);
}

static void counter_init(const void *dev)
{
	gpt_config_t gptConfig;
	IRQn_Type irqn = gpt_get_irqn(dev);
	struct counter_alarm *counter;
	int ret;

	os_assert(dev != NULL, "Null pointer!");
	os_assert(irqn != NotAvail_IRQn, "Unknown IRQn for device %p", dev);

	GPT_GetDefaultConfig(&gptConfig);
	gptConfig.enableFreeRun = true;
	gptConfig.clockSource = kGPT_ClockSource_Periph;
	gptConfig.divider = 1;
	GPT_Init((GPT_Type *)(dev), &gptConfig);

	ret = irq_register(irqn, gpt_irq_handler, (void *)dev);
	os_assert(!ret, "Failed to register counter's IRQ! (%d)", ret);
	EnableIRQ(irqn);

	counter = &counters[gpt_get_index(dev)];
	counter->dev = dev;

	log_debug("counter %d using GPT dev %p irq %d initialized\n",
			gpt_get_index(dev), dev, irqn);
}

int os_counter_start(const void *dev)
{
	uint8_t index = gpt_get_index(dev);
	struct counter_alarm *counter = &counters[index];

	os_assert(index != 0, "Unknown device %p", dev);

	if (!counter->dev)
		counter_init(dev);

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
	return (1000 * (uint64_t)ticks) / SOURCE_CLOCK_FREQ_MHZ;
}

uint32_t os_counter_get_top_value(const void *dev)
{
	return UINT32_MAX;
}

uint8_t os_counter_get_num_of_channels(const void *dev)
{
	return NB_CHANNELS;
}

/*
 * After expiration alarm can be set again, disabling is not needed.
 * When alarm expiration handler is called, channel is considered available and can be set again in that context.
 */
int os_counter_set_channel_alarm(const void *dev, uint8_t chan_id,
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

	GPT_EnableInterrupts((GPT_Type *)dev, kGPT_OutputCompare1InterruptEnable);
	GPT_SetOutputCompareValue((GPT_Type *)dev, kGPT_OutputCompare_Channel1, next);

exit:
	return ret;
}

