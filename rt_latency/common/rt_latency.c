/*
 * Copyright 2021-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/counter.h"
#include "os/cache.h"

#include "rtos_apps/log.h"
#include "rtos_apps/stats.h"

#include "hrpn_ctrl.h"
#include "rt_latency.h"
#include "rtos_abstraction_layer.h"

#define EPT_ADDR (30)

static inline uint32_t calc_diff_ns(os_counter_t *dev,
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
static void latency_alarm_handler(os_counter_t *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	struct rt_latency_ctx *ctx = user_data;
	bool yield = false;
	int err;

	ctx->time_irq = irq_counter;

	err = rtos_sem_give_from_isr(&ctx->semaphore, &yield);
	rtos_assert(!err, "Failed to give semaphore from isr (err: %d)", err);

	rtos_yield_from_isr(yield);
}

#define IRQ_LOAD_ISR_DURATION_US	10

static void load_alarm_handler(os_counter_t *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	struct rt_latency_ctx *ctx = user_data;
	uint32_t start, cur;
	bool yield = false;
	int err;

	os_counter_get_value(dev, &start);

	do {
		os_counter_get_value(dev, &cur);
	} while (calc_diff_ns(dev, start, cur) < IRQ_LOAD_ISR_DURATION_US * 1000);

	os_counter_stop(dev);

	err = rtos_sem_give_from_isr(&ctx->irq_load_sem, &yield);
	rtos_assert(!err, "Failed to give semaphore from isr (err: %d)", err);

	rtos_yield_from_isr(yield);
}

static void rt_latency_stats_dump(struct rt_latency_ctx *ctx)
{
	if (!ctx->stats_snapshot.pending) {
		memcpy(&ctx->stats_snapshot, &ctx->stats, sizeof(struct rt_latency_stats));

		ctx->stats_snapshot.pending = true;

		stats_reset(&ctx->stats.irq_delay);
		stats_reset(&ctx->stats.irq_to_sched);
	}
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
	os_counter_t *dev = ctx->dev;
	static uint32_t ticks = 0;
#define LATENCY_STATS_PERIOD (LATENCY_STATS_PERIOD_SEC * 1000000 / COUNTER_PERIOD_US_VAL)
	static uint64_t stats_cnt = 0;

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
		rtos_assert(!err, "Counter set alarm failed (err: %d)", err);
		err = rtos_sem_init(&ctx->irq_load_sem, 0);
		rtos_assert(!err, "semaphore init failed!");
	}

retry:
	/* Configure IRQ latency testing alarm */
	os_counter_get_value(dev, &cnt);
	alarm_cfg.ticks = cnt + ticks;
	alarm_cfg.flags = OS_COUNTER_ALARM_CFG_ABSOLUTE;
	alarm_cfg.user_data = ctx;
	alarm_cfg.callback = latency_alarm_handler;

	ctx->time_prog = alarm_cfg.ticks;

	err = os_counter_set_channel_alarm(dev, 0, &alarm_cfg);
	rtos_assert(!err, "Counter set alarm failed (err: %d)", err);

	/* Sync current thread with alarm callback function thanks to a semaphore */
	err = rtos_sem_take(&ctx->semaphore, RTOS_MS_TO_TICKS(COUNTER_IRQ_TIMEOUT_MS));
	if (err < 0) {
		/* waiting period timed out: probably late alarm scheduling and waiting for counter wrap. */
		os_counter_cancel_channel_alarm(dev, 0);
		/* There is a small race window between cancel alarm and alarm callback
		 * giving the semaphore. But the difference between the small timeout
		 * and the time for timer (few minutes at 24Mhz) to wrap covers that.
		 */
		ctx->stats.late_alarm_sched++;
		goto retry;
	}

	/* Woken up... fetch counter value to compute latency */
	os_counter_get_value(dev, &now);
	os_counter_stop(dev);

	irq_delay = calc_diff_ns(dev, ctx->time_prog, ctx->time_irq);

	stats_update(&ctx->stats.irq_delay, irq_delay);
	hist_update(&ctx->stats.irq_delay_hist, irq_delay);

	irq_to_sched = calc_diff_ns(dev, ctx->time_prog, now);
	stats_update(&ctx->stats.irq_to_sched, irq_to_sched);
	hist_update(&ctx->stats.irq_to_sched_hist, irq_to_sched);

	if (!ctx->quiet) {
		/* Dump statistics every TIMER_STATS_PERIOD_SEC seconds */
		if (!(++stats_cnt % LATENCY_STATS_PERIOD))
			rt_latency_stats_dump(ctx);
	}

	if (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD) {
		/* Waiting irq load ISR exits and then go to next loop */
		err = rtos_sem_take(&ctx->irq_load_sem, RTOS_WAIT_FOREVER);
		rtos_assert(!err, "Can't take the semaphore (err: %d)", err);
	}

	return err;
}

void cpu_load(struct rt_latency_ctx *ctx)
{
	int err;

	if (ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD_SEM) {
		err = rtos_sem_take(&ctx->cpu_load_sem, RTOS_WAIT_FOREVER);
		rtos_assert(!err, "Failed to take semaphore");

		err = rtos_sem_give(&ctx->cpu_load_sem);
		rtos_assert(!err, "Failed to give semaphore");
	}
}

void cache_inval(void)
{
	os_dcache_invd_all();
	os_icache_invd_all();

	rtos_sleep(RTOS_MS_TO_TICKS(CACHE_INVAL_PERIOD_MS));
}

void print_stats(struct rt_latency_ctx *ctx)
{
	if (ctx->stats_snapshot.pending) {
		stats_compute(&ctx->stats_snapshot.irq_delay);
		stats_print(&ctx->stats_snapshot.irq_delay);
		hist_print(&ctx->stats_snapshot.irq_delay_hist);

		stats_compute(&ctx->stats_snapshot.irq_to_sched);
		stats_print(&ctx->stats_snapshot.irq_to_sched);
		hist_print(&ctx->stats_snapshot.irq_to_sched_hist);
		log_info("late alarm scheduling: %u\n", ctx->stats_snapshot.late_alarm_sched);
		log_info("\n");

		ctx->stats_snapshot.pending = false;
	}
}

void rt_latency_destroy(struct rt_latency_ctx *ctx)
{
	int err;
	os_counter_t *dev = ctx->dev;

	if (ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		rtos_sem_destroy(&ctx->cpu_load_sem);
	}

	if (ctx->tc_load & RT_LATENCY_WITH_IRQ_LOAD) {
		err = os_counter_stop(ctx->irq_load_dev);
		rtos_assert(!err, "Failed to stop counter!");

		err = os_counter_cancel_channel_alarm(ctx->irq_load_dev, 0);
		rtos_assert(!err, "Failed to cancel counter alarm!");

		rtos_sem_destroy(&ctx->irq_load_sem);
	}

	err = os_counter_stop(dev);
	rtos_assert(!err, "Failed to stop counter!");

	err = os_counter_cancel_channel_alarm(dev, 0);
	rtos_assert(!err, "Failed to cancel counter alarm!");

	rtos_sem_destroy(&ctx->semaphore);

	/* dump and print current stats before reseting them all */
	rt_latency_stats_dump(ctx);
	print_stats(ctx);

	stats_reset(&ctx->stats.irq_delay);
	hist_reset(&ctx->stats.irq_delay_hist);

	stats_reset(&ctx->stats.irq_to_sched);
	hist_reset(&ctx->stats.irq_to_sched_hist);

	ctx->stats.late_alarm_sched = 0;

	ctx->dev = NULL;
	ctx->irq_load_dev = NULL;
}

int rt_latency_init(os_counter_t *dev,
		os_counter_t *irq_load_dev, struct rt_latency_ctx *ctx)
{
	int err;

	ctx->dev = dev;
	ctx->irq_load_dev = irq_load_dev;

	stats_init(&ctx->stats.irq_delay, 31, "irq delay (ns)", NULL);
	hist_init(&ctx->stats.irq_delay_hist, 20, 1000);

	stats_init(&ctx->stats.irq_to_sched, 31, "irq to sched (ns)", NULL);
	hist_init(&ctx->stats.irq_to_sched_hist, 20, 1000);

	ctx->stats_snapshot.pending = false;

	ctx->stats.late_alarm_sched = 0;

	err = rtos_sem_init(&ctx->semaphore, 0);
	rtos_assert(!err, "semaphore creation failed!");

	if (ctx->tc_load & RT_LATENCY_WITH_CPU_LOAD) {
		err = rtos_sem_init(&ctx->cpu_load_sem, 0);
		rtos_assert(!err, "semaphore init failed!");
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

static void response(struct rpmsg_ept *ept, uint32_t status)
{
	struct hrpn_resp_latency resp;

	resp.type = HRPN_RESP_TYPE_LATENCY;
	resp.status = status;
	rpmsg_send(ept, &resp, sizeof(resp));
}

void command_handler(void *ctx, struct rpmsg_ept *ept)
{
	struct hrpn_command cmd;
	unsigned int len;
	int ret;

	len = sizeof(cmd);
	if (rpmsg_recv(ept, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_LATENCY_RUN:
		if (len != sizeof(struct hrpn_cmd_latency_run)) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		if (cmd.u.latency_run.id >= RT_LATENCY_TEST_CASE_MAX) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		ret = start_test_case(ctx, cmd.u.latency_run.id, cmd.u.latency_run.quiet);
		if (ret)
			response(ept, HRPN_RESP_STATUS_ERROR);
		else
			response(ept, HRPN_RESP_STATUS_SUCCESS);

		break;

	case HRPN_CMD_TYPE_LATENCY_STOP:
		if (len != sizeof(struct hrpn_cmd_latency_stop)) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		destroy_test_case(ctx);
		response(ept, HRPN_RESP_STATUS_SUCCESS);
		break;

	default:
		response(ept, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

int ctrl_ctx_init(struct ctrl_ctx *ctrl)
{
	int rc = 0;

	ctrl->ept = rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw");
	rtos_assert(ctrl->ept, "rpmsg transport initialization failed, cannot proceed\n");

	return rc;
}
