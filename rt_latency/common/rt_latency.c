/*
 * Copyright 2021-2022 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/counter.h"
#include "os/cache.h"
#include "os/semaphore.h"
#include "os/unistd.h"

#include "hlog.h"
#include "stats.h"

#include "hrpn_ctrl.h"
#include "mailbox.h"
#include "rt_latency.h"
#include "rpmsg.h"

#define EPT_ADDR (30)

static inline uint32_t calc_diff_ns(const void *dev,
			uint32_t cnt_1, uint32_t cnt_2)
{
	uint32_t diff;
	static uint32_t top = 0;
	static int counting_up = -1;

	/*
	 * Set these variables once for all to avoid multiple register accesses
	 * each time we call this function.
	 */
	if (top == 0)
		top = os_counter_get_top_value(dev);

	if (counting_up == -1)
		counting_up = os_counter_is_counting_up(dev);

	if (counting_up) {
		diff =  (cnt_2 < cnt_1) ?
			(cnt_2 + top - cnt_1) : (cnt_2 - cnt_1);
	} else {
		diff = (cnt_2 > cnt_1) ?
			(cnt_1 + top - cnt_2) : (cnt_1 - cnt_2);
	}

	return os_counter_ticks_to_ns(dev, diff);
}

/*
 * Used by all RTOS:
 *    o FreeRTOS through IRQ_Handler_GPT() handler
 *    o Zephyr through the alarm's ->callback
 *
 * @current_counter: counter value when the IRQ occurred
 */
static void latency_alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	struct rt_latency_ctx *ctx = user_data;

	ctx->time_irq = irq_counter;

	os_sem_give(&ctx->semaphore, OS_SEM_FLAGS_ISR_CONTEXT);
}

#define IRQ_LOAD_ISR_DURATION_US	10

static void load_alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	uint32_t start, cur;
	struct rt_latency_ctx *ctx = user_data;

	os_counter_get_value(dev, &start);

	do {
		os_counter_get_value(dev, &cur);
	} while (calc_diff_ns(dev, start, cur) < IRQ_LOAD_ISR_DURATION_US * 1000);

	os_counter_stop(dev);
	os_sem_give(&ctx->irq_load_sem, OS_SEM_FLAGS_ISR_CONTEXT);
}

/*
 * Blocking function including an infinite loop ;
 * must be called by separate threads/tasks.
 *
 * In case an error is returned the caller shall handle the lifecycle of the
 * thread by suspending or destroying the latter.
 */
int rt_latency_test(struct rt_latency_ctx *ctx)
{
	int err;
	uint32_t cnt;
	struct os_counter_alarm_cfg alarm_cfg;
	struct os_counter_alarm_cfg load_alarm_cfg;
	uint32_t now;
	uint64_t irq_delay;
	uint64_t irq_to_sched;
	const void *dev = ctx->dev;
	static uint32_t ticks = 0;

	/* only compute it once for all */
	if (!ticks) {
		uint32_t counter_period_us;

		counter_period_us = COUNTER_PERIOD_US_VAL;
		ticks = os_counter_us_to_ticks(dev, counter_period_us);
	}

	/* Start IRQ latency testing alarm */
	os_counter_start(dev);

	/* Start IRQ load alarm, if need be */
	if (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD) {
		os_counter_start(ctx->irq_load_dev);
		cnt = COUNTER_PERIOD_US_VAL - 1;
		load_alarm_cfg.ticks = os_counter_us_to_ticks(ctx->irq_load_dev, cnt);
		load_alarm_cfg.flags = 0;
		load_alarm_cfg.user_data = ctx;
		load_alarm_cfg.callback = load_alarm_handler;

		err = os_counter_set_channel_alarm(ctx->irq_load_dev, 0, &load_alarm_cfg);
		os_assert(!err, "Counter set alarm failed (err: %d)", err);
		err = os_sem_init(&ctx->irq_load_sem, 0);
		os_assert(!err, "semaphore init failed!");
	}

	/* Configure IRQ latency testing alarm */
	os_counter_get_value(dev, &cnt);
	alarm_cfg.ticks = cnt + ticks;
	alarm_cfg.flags = OS_COUNTER_ALARM_CFG_ABSOLUTE;
	alarm_cfg.user_data = ctx;
	alarm_cfg.callback = latency_alarm_handler;

	ctx->time_prog = alarm_cfg.ticks;

	err = os_counter_set_channel_alarm(dev, 0, &alarm_cfg);
	os_assert(!err, "Counter set alarm failed (err: %d)", err);

	/* Sync current thread with alarm callback function thanks to a semaphore */
	err = os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	os_assert(!err, "Can't take the semaphore (err: %d)", err);

	/* Woken up... calculate latency */
	os_counter_get_value(dev, &now);

	irq_delay = calc_diff_ns(dev, ctx->time_prog, ctx->time_irq);

	stats_update(&ctx->irq_delay, irq_delay);
	hist_update(&ctx->irq_delay_hist, irq_delay);

	irq_to_sched = calc_diff_ns(dev, ctx->time_prog, now);
	stats_update(&ctx->irq_to_sched, irq_to_sched);
	hist_update(&ctx->irq_to_sched_hist, irq_to_sched);

	os_counter_stop(dev);

	if (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD) {
		/* Waiting irq load ISR exits and then go to next loop */
		err = os_sem_take(&ctx->irq_load_sem, 0, OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Can't take the semaphore (err: %d)", err);
	}

	return err;
}

void cpu_load(struct rt_latency_ctx *ctx)
{
	int err;

	if (ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD_SEM) {
		err = os_sem_take(&ctx->cpu_load_sem, 0, OS_SEM_TIMEOUT_MAX);
		os_assert(!err, "Failed to take semaphore");

		err = os_sem_give(&ctx->cpu_load_sem, 0);
		os_assert(!err, "Failed to give semaphore");
	}
}

void cache_inval(void)
{
	os_dcache_invd_all();
	os_icache_invd_all();

	os_msleep(CACHE_INVAL_PERIOD_MS);
}

void print_stats(struct rt_latency_ctx *ctx)
{
	stats_compute(&ctx->irq_delay);
	stats_print(&ctx->irq_delay);
	stats_reset(&ctx->irq_delay);
	hist_print(&ctx->irq_delay_hist);

	stats_compute(&ctx->irq_to_sched);
	stats_print(&ctx->irq_to_sched);
	stats_reset(&ctx->irq_to_sched);
	hist_print(&ctx->irq_to_sched_hist);

	log_info("\n");
}

void rt_latency_destroy(struct rt_latency_ctx *ctx)
{
	int err;
	const void *dev = ctx->dev;

	if (ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		err = os_sem_destroy(&ctx->cpu_load_sem);
		os_assert(!err, "semaphore init failed!");
	}

	if (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD) {
		err = os_counter_stop(ctx->irq_load_dev);
		os_assert(!err, "Failed to stop counter!");

		err = os_counter_cancel_channel_alarm(ctx->irq_load_dev, 0);
		os_assert(!err, "Failed to cancel counter alarm!");

		err = os_sem_destroy(&ctx->irq_load_sem);
		os_assert(!err, "Failed to destroy semaphore!");
	}

	err = os_counter_stop(dev);
	os_assert(!err, "Failed to stop counter!");

	err = os_counter_cancel_channel_alarm(dev, 0);
	os_assert(!err, "Failed to cancel counter alarm!");

	err = os_sem_destroy(&ctx->semaphore);
	os_assert(!err, "Failed to destroy semaphore!");

	/* print current stats before reseting them all */
	print_stats(ctx);

	stats_reset(&ctx->irq_delay);
	hist_reset(&ctx->irq_delay_hist);

	stats_reset(&ctx->irq_to_sched);
	hist_reset(&ctx->irq_to_sched_hist);

	ctx->dev = NULL;
	ctx->irq_load_dev = NULL;
}

int rt_latency_init(const void *dev,
		const void *irq_load_dev, struct rt_latency_ctx *ctx)
{
	int err;

	ctx->dev = dev;
	ctx->irq_load_dev = irq_load_dev;

	stats_init(&ctx->irq_delay, 31, "irq delay (ns)", NULL);
	hist_init(&ctx->irq_delay_hist, 20, 1000);

	stats_init(&ctx->irq_to_sched, 31, "irq to sched (ns)", NULL);
	hist_init(&ctx->irq_to_sched_hist, 20, 1000);

	err = os_sem_init(&ctx->semaphore, 0);
	os_assert(!err, "semaphore creation failed!");

	if (ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		err = os_sem_init(&ctx->cpu_load_sem, 0);
		os_assert(!err, "semaphore init failed!");
	}

	if (ctx->tc_load & RT_LATENCY_WITH_LINUX_LOAD) {
		/* TODO: Add command to trigger Linux Load */
		log_warn("Linux load must be run manually!\n");
	}

	if (ctx->tc_load & RT_LATENCY_USES_OCRAM) {
		/* TODO: Place specific code/data in OCRAM */
		log_warn("not supported\n");
	}

	return err;
}

static void response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_latency resp;

	resp.type = HRPN_RESP_TYPE_LATENCY;
	resp.status = status;
	mailbox_resp_send(m, &resp, sizeof(resp));
}

void command_handler(void *ctx, struct mailbox *m)
{
	struct hrpn_command cmd;
	unsigned int len;
	int ret;

	len = sizeof(cmd);
	if (mailbox_cmd_recv(m, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_LATENCY_RUN:
		if (len != sizeof(struct hrpn_cmd_latency_run)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		if (cmd.u.latency_run.id >= RT_LATENCY_TEST_CASE_MAX) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		ret = start_test_case(ctx, cmd.u.latency_run.id);
		if (ret)
			response(m, HRPN_RESP_STATUS_ERROR);
		else
			response(m, HRPN_RESP_STATUS_SUCCESS);

		break;

	case HRPN_CMD_TYPE_LATENCY_STOP:
		if (len != sizeof(struct hrpn_cmd_latency_stop)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		destroy_test_case(ctx);
		response(m, HRPN_RESP_STATUS_SUCCESS);
		break;

	default:
		response(m, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

int ctrl_ctx_init(struct ctrl_ctx *ctrl)
{
	void *cmd, *resp;
	void *tp = NULL;
	int rc;

	rc = rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw",
				  &tp, &cmd, &resp);
	os_assert(!rc, "rpmsg transport initialization failed, cannot proceed\n");
	rc = mailbox_init(&ctrl->mb, cmd, resp, false, tp);
	os_assert(!rc, "mailbox initialization failed!");

	return rc;
}
