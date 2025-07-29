/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/counter.h"
#include "os/irq.h"

#include "rtos_apps/log.h"
#include "rtos_abstraction_layer.h"

#include "fsl_device_registers.h"
#include "fsl_tpm.h"

/* FIXME use fsl_clock to get the frequency */
#define SOURCE_CLOCK_FREQ_HZ	24000000

static int os_counter_tpm_get_value(const os_counter_t *dev, uint32_t *cnt);

static int set_alarm(os_counter_t *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm_cfg)
{
	struct os_counter_alarm_cfg *alarm = &dev->alarms[chan_id];

	rtos_assert(dev->initialized == true, "TPM device %p not initialized!", dev->base);

	/* Fail if alarm already set for this channel. */
	if (alarm->callback)
		return -1;

	/* save the alarm config */
	alarm->callback = alarm_cfg->callback;
	alarm->user_data = alarm_cfg->user_data;

	return 0;
}

static void reset_alarm(os_counter_t *dev, uint8_t chan_id)
{
	struct os_counter_alarm_cfg *alarm = &dev->alarms[chan_id];

	/* reset alarm config */
	alarm->callback = NULL;
}

static void tpm_irq_ack(os_counter_t *dev, uint8_t chan_id)
{
	/* TODO: support multiple channels */
	if (chan_id != kTPM_Chnl_0) {
		/* TODO: support multiple channels */
		log_err("Channel ID (%d) not supported!\n", chan_id);

	} else {
		TPM_DisableInterrupts((TPM_Type *)dev->base, kTPM_Chnl0InterruptEnable);
		TPM_ClearStatusFlags((TPM_Type *)dev->base, kTPM_Chnl0Flag);
	}
}

static void tpm_irq_handler(void *irq_dev)
{
	os_counter_t *dev = (os_counter_t *)irq_dev;
	const struct os_counter_alarm_cfg *alarm;
	uint8_t chan_id = kTPM_Chnl_0; /* TODO support multiple channels*/
	uint32_t now;
	int ret;

	ret = os_counter_tpm_get_value(dev, &now);
	if (ret)
		assert(true);

	tpm_irq_ack(dev, chan_id);

	alarm = &dev->alarms[chan_id];

	if (alarm->callback)
		alarm->callback(dev, chan_id, now, alarm->user_data);

	reset_alarm(dev, chan_id);
}

static void counter_init(os_counter_t *dev)
{
	IRQn_Type irqn = dev->irqn;
	tpm_config_t tpmConfig;
	TPM_Type *base = (TPM_Type *)dev->base;
	int ret;

	TPM_GetDefaultConfig(&tpmConfig);
	/* Use a prescaler devide by 1. */
	tpmConfig.prescale = kTPM_Prescale_Divide_1;
	TPM_Init(base, &tpmConfig);

	/* Set the modulo to max value. */
	base->MOD = TPM_MAX_COUNTER_VALUE(base);

	ret = os_irq_register(irqn, tpm_irq_handler, (void *)dev, dev->irq_prio);
	rtos_assert(!ret, "Failed to register counter's IRQ! (%d)", ret);
	os_irq_enable(irqn);

	dev->initialized = true;

	log_debug("using TPM dev %p irq %d initialized\n",
			base, irqn);
}

static int os_counter_tpm_start(os_counter_t *dev)
{
	if (!dev->initialized)
		counter_init(dev);

	TPM_StartTimer((TPM_Type *)dev->base, kTPM_SystemClock);

	return 0;
}

static int os_counter_tpm_stop(const os_counter_t *dev)
{
	TPM_StopTimer((TPM_Type *)dev->base);

	return 0;
}

static int os_counter_tpm_get_value(const os_counter_t *dev, uint32_t *cnt)
{
	if (!cnt)
		return -1;

	*cnt = TPM_GetCurrentTimerCount((TPM_Type *)dev->base);

	return 0;
}

static bool os_counter_tpm_is_counting_up(const os_counter_t *dev)
{
	return true;
}

static uint32_t os_counter_tpm_get_frequency(const os_counter_t *dev)
{
	return SOURCE_CLOCK_FREQ_HZ;
}

static uint32_t os_counter_tpm_get_top_value(const os_counter_t *dev)
{
	return TPM_MAX_COUNTER_VALUE((TPM_Type *)dev->base);
}

static uint8_t os_counter_tpm_get_num_of_channels(const os_counter_t *dev)
{
	return NB_CHANNELS;
}

/*
 * After expiration alarm can be set again, disabling is not needed.
 * When alarm expiration handler is called, channel is considered available and can be set again in that context.
 */
static int os_counter_tpm_set_channel_alarm(os_counter_t *dev, uint8_t chan_id,
		const struct os_counter_alarm_cfg *alarm_cfg)
{
	int ret = 0;
	uint32_t current, next;

	if (!alarm_cfg) {
		log_err("Null pointer for channel ID (%d)\n", chan_id);

		ret = -1;
		goto exit;
	}

	if (chan_id != kTPM_Chnl_0) {
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
		os_counter_tpm_get_value(dev, &current);
		next += current;
	}

	TPM_EnableInterrupts((TPM_Type *)dev->base, kTPM_Chnl0InterruptEnable);
	TPM_SetupOutputCompare((TPM_Type *)dev->base, kTPM_Chnl_0, kTPM_NoOutputSignal, next);

exit:
	return ret;
}

static int os_counter_tpm_cancel_channel_alarm(os_counter_t *dev, uint8_t chan_id)
{
	int ret = 0;

	if (chan_id != kTPM_Chnl_0) {
		/* TODO: support multiple channels */
		log_err("Channel ID (%d) not supported!\n", chan_id);

		ret = -1;
		goto exit;
	}

	TPM_DisableInterrupts((TPM_Type *)dev->base, kTPM_Chnl0InterruptEnable);
	TPM_ClearStatusFlags((TPM_Type *)dev->base, kTPM_Chnl0Flag);

	reset_alarm(dev, chan_id);

exit:
	return ret;
}

#if defined(BOARD_COUNTER_0_BASE) || defined(BOARD_COUNTER_1_BASE)
static const struct os_counter_ops tpm_counter_ops = {
	.os_counter_start = os_counter_tpm_start,
	.os_counter_stop = os_counter_tpm_stop,
	.os_counter_get_num_of_channels = os_counter_tpm_get_num_of_channels,
	.os_counter_get_value = os_counter_tpm_get_value,
	.os_counter_is_counting_up = os_counter_tpm_is_counting_up,
	.os_counter_get_frequency = os_counter_tpm_get_frequency,
	.os_counter_get_top_value = os_counter_tpm_get_top_value,
	.os_counter_set_channel_alarm = os_counter_tpm_set_channel_alarm,
	.os_counter_cancel_channel_alarm = os_counter_tpm_cancel_channel_alarm,
};
#endif

#if defined(BOARD_COUNTER_0_BASE) && defined(BOARD_COUNTER_0_IRQ) && defined(BOARD_COUNTER_0_IRQ_PRIO)
os_counter_t freertos_counter_instance_0 = {
	.base = BOARD_COUNTER_0_BASE,
	.irqn = BOARD_COUNTER_0_IRQ,
	.irq_prio = BOARD_COUNTER_0_IRQ_PRIO,
	.ops = &tpm_counter_ops,
};
#endif

#if defined(BOARD_COUNTER_1_BASE) && defined(BOARD_COUNTER_1_IRQ) && defined(BOARD_COUNTER_1_IRQ_PRIO)
os_counter_t freertos_counter_instance_1 = {
	.base = BOARD_COUNTER_1_BASE,
	.irqn = BOARD_COUNTER_1_IRQ,
	.irq_prio = BOARD_COUNTER_1_IRQ_PRIO,
	.ops = &tpm_counter_ops,
};
#endif
