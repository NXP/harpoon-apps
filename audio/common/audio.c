/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hlog.h"
#include "os/assert.h"
#include "os/mqueue.h"
#include "os/semaphore.h"
#include "os/stdlib.h"
#include "os/unistd.h"
#include "os/cpu_load.h"

#include "rpmsg.h"
#include "hrpn_ctrl.h"

#include "audio.h"
#include "audio_entry.h"

#include "app_board.h"
#include "codec_config.h"
#include "sai_clock_config.h"
#include "sai_drv.h"
#include "sai_config.h"

#include "audio_pipeline.h"

#define EPT_ADDR	(30)

#if (CONFIG_GENAVB_ENABLE == 1)
#include "system_config.h"
#endif

struct mode_handler {
	void *(*init)(void *);
	void (*exit)(void *);
	void (*stats)(void *);
	void (*ctrl)(void *);
	int (*run)(void *, struct event *e);
	void *data;
};

#define DEFAULT_PERIOD		8
#define DEFAULT_SAMPLE_RATE	48000
#define USE_TX_IRQ		1
#define SAI_DEV_IRQ_SOURCE	1

static const int supported_period[] = {2, 4, 8, 16, 32};
static const uint32_t supported_rate[] = {44100, 48000, 88200, 96000, 176400, 192000};

struct ctrl_ctx {
	struct rpmsg_ept *ept;
};

struct data_ctx {
	uint8_t thread_count;
	uint8_t pipeline_count;

	/* SAI data for hardware setup */
	struct sai_device dev[SAI_TX_MAX_INSTANCE];
	sai_word_width_t bit_width;
	sai_sample_rate_t sample_rate;
	uint32_t chan_numbers;
	uint8_t period;

	uint64_t callback;

	/* The first thread is used for parent pipeline, others are for child pipeline */
	struct thread_data_ctx_t {
		os_sem_t semaphore;
		os_sem_t async_sem;
		os_mqd_t mqueue;
		/* pipeline_ctx handle for current thread */
		void *handle;
	} thread_data_ctx[MAX_AUDIO_DATA_THREADS];

	struct ctrl_ctx ctrl;
	const struct mode_handler *handler;
	os_sem_t reset_sem;
};

static struct play_pipeline_config play_pipeline_dtmf_config = {
	.cfg = {
		&pipeline_dtmf_config,
	}
};

static struct play_pipeline_config play_pipeline_loopback_config = {
	.cfg = {
		&pipeline_loopback_config,
	}
};

static struct play_pipeline_config play_pipeline_sine_config = {
	.cfg = {
		&pipeline_sine_config,
	}
};

static struct play_pipeline_config play_pipeline_full_config = {
	.cfg = {
		&pipeline_full_config,
	}
};

#ifdef CONFIG_SMP
static struct play_pipeline_config play_pipeline_smp_config = {
	.cfg = {
		&pipeline_full_thread_0_config,
		&pipeline_full_thread_1_config,
	}
};
#endif

#if (CONFIG_GENAVB_ENABLE == 1)
static struct play_pipeline_config play_pipeline_full_avb_config = {
	.cfg = {
		&pipeline_full_avb_config,
	}
};
static struct play_pipeline_config play_pipeline_mcr_avb_config = {
	.cfg = {
		&pipeline_mcr_avb_config,
	}
};
#endif

const static struct mode_handler g_handler[] =
{
	[0] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.data = &play_pipeline_dtmf_config,
	},
	[1] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.data = &play_pipeline_sine_config,
	},
	[2] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.data = &play_pipeline_loopback_config,
	},
	[3] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.data = &play_pipeline_full_config,
	},
#if (CONFIG_GENAVB_ENABLE == 1)
	[4] = {
		.init = play_pipeline_init_avb,
		.exit = play_pipeline_exit_avb,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.ctrl = play_pipeline_ctrl_avb,
		.data = &play_pipeline_full_avb_config,
	},
#endif
#ifdef CONFIG_SMP
	[5] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.data = &play_pipeline_smp_config,
	}
#endif
#if (CONFIG_GENAVB_ENABLE == 1)
	[6] = {
		.init = play_pipeline_init_avb,
		.exit = play_pipeline_exit_avb,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.ctrl = play_pipeline_ctrl_avb,
		.data = &play_pipeline_mcr_avb_config,
	}
#endif
};

static void audio_check_params(struct audio_config *cfg, int run_id)
{
	log_debug("enter\n");

	if (cfg->period == 2) {
		switch (cfg->rate) {
		case 176400:
		case 192000:
			log_warn("Unsupported rate(%d Hz)/period(%d) combination\n", cfg->rate, cfg->period);
			break;
		default:
			break;
		}
	}
}

static void audio_set_hw_addr(struct audio_config *cfg, uint8_t *hw_addr)
{
	uint8_t *addr = cfg->address;

	memcpy(addr, hw_addr, sizeof(cfg->address));

	log_info("%02x:%02x:%02x:%02x:%02x:%02x\n",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static void data_send_event(struct data_ctx *ctx, uint8_t status)
{
	struct event e;
	int i;

	e.type = EVENT_TYPE_DATA;
	e.data = status;

	for (i = 0; i < ctx->pipeline_count; i++)
		os_mq_send(&ctx->thread_data_ctx[i].mqueue, &e, OS_MQUEUE_FLAGS_ISR_CONTEXT, 0);
}

static void rx_callback(uint8_t status, void *user_data)
{
	struct data_ctx *ctx = (struct data_ctx *)user_data;

#if USE_TX_IRQ
	sai_disable_irq(&ctx->dev[SAI_DEV_IRQ_SOURCE], false, true);
#else
	sai_disable_irq(&ctx->dev[SAI_DEV_IRQ_SOURCE], true, false);
#endif

	ctx->callback++;

	data_send_event(ctx, status);
}

static void pll_adjust_disable(struct data_ctx *ctx)
{
	struct hrpn_cmd_audio_element_pll cmd;
	int i;

	/* need to disable PLL audio element */
	cmd.u.common.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_DISABLE;
	cmd.u.common.element.type = AUDIO_ELEMENT_PLL;
	cmd.u.common.element.id = 0;

	for (i = 0; i < MAX_PIPELINES; i++) {
		cmd.u.common.pipeline.id = i;
		audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&cmd, sizeof(cmd), NULL);
	}
}

static void pll_adjust_set_pll_id(struct data_ctx *ctx, uint32_t pll_id)
{
	struct hrpn_cmd_audio_element_pll cmd;
	int i;

	/* PLL element needs to know the sampling rate to determine the input PLL */
	cmd.u.common.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ID;
	cmd.u.common.element.type = AUDIO_ELEMENT_PLL;
	cmd.u.common.element.id = 0;
	cmd.pll_id = pll_id;

	for (i = 0; i < MAX_PIPELINES; i++) {
		cmd.u.common.pipeline.id = i;
		audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&cmd, sizeof(cmd), NULL);
	}
}

static void sai_setup(struct data_ctx *ctx)
{
	struct sai_cfg sai_config;
	bool pll_disable = true;
	int i;

	log_info("enter\n");

	/* Configure each active SAI */
	for (i = 0; i < sai_active_list_nelems; i++) {
		uint32_t pll_id;
		int sai_id;
		enum codec_id cid;
		int32_t ret;

		sai_config.sai_base = sai_active_list[i].sai_base;
		sai_config.bit_width = ctx->bit_width;
		sai_config.sample_rate = ctx->sample_rate;
		sai_config.chan_numbers = ctx->chan_numbers;

		sai_id = get_sai_id(sai_active_list[i].sai_base);
		os_assert(sai_id, "SAI%d enabled but not supported in this platform!", i);

		sai_config.source_clock_hz = get_sai_clock_freq(i);

		sai_config.tx_sync_mode = sai_active_list[i].tx_sync_mode;
		sai_config.rx_sync_mode = sai_active_list[i].rx_sync_mode;
		sai_config.msel = sai_active_list[i].msel;

		if (i == SAI_DEV_IRQ_SOURCE) {
			/* SAI instance used as IRQ source */
			sai_config.rx_callback = rx_callback;
			sai_config.rx_user_data = ctx;
			sai_config.working_mode = SAI_RX_IRQ_MODE;
		} else {
			sai_config.rx_callback = NULL;
			sai_config.rx_user_data = NULL;
			sai_config.working_mode = SAI_POLLING_MODE;
		}

		/* Configure attached codec */
		cid = sai_active_list[i].cid;
		ret = codec_setup(cid);
		if (ret != kStatus_Success) {
			if ((i == 0) && (sai_active_list[i].masterSlave == kSAI_Slave)) {
				/* SAI 5 cannot be slave if no CODEC is connected */
				log_info("No codec found on SAI%d, forcing master mode\n",
						get_sai_id(sai_active_list[i].sai_base));
				sai_active_list[i].masterSlave = kSAI_Master;
			}
		} else {
			codec_set_format(cid, sai_config.source_clock_hz, sai_config.sample_rate, ctx->bit_width);
		}

		sai_config.masterSlave = sai_active_list[i].masterSlave;

		if (sai_config.masterSlave == kSAI_Slave)
			pll_disable = false;

		/* Set FIFO water mark to be period size of all channels*/
#if USE_TX_IRQ
		sai_config.fifo_water_mark = ctx->period * ctx->chan_numbers - 1;
#else
		sai_config.fifo_water_mark = ctx->period * ctx->chan_numbers;
#endif

		sai_drv_setup(&ctx->dev[i], &sai_config);

		pll_id = sai_select_audio_pll_mux(sai_id, sai_config.sample_rate);
		pll_adjust_set_pll_id(ctx, pll_id);
	}

	if (pll_disable)
		pll_adjust_disable(ctx);
}

static void sai_close(struct data_ctx *ctx)
{
	int i;

	/* Close each active SAI */
	for (i = 0; i < sai_active_list_nelems; i++) {
		sai_drv_exit(&ctx->dev[i]);
	}
}

static void audio_reset(struct data_ctx *ctx, unsigned int id)
{
	struct event e;
	int i, ret;

	/*
	 *  If reset process has already been raised by another pipeline, return immediately
	 *  because reset process will cover all pipeline.
	 */
	ret = os_sem_take(&ctx->reset_sem, 0, 0);
	if (ret)
		return;

	e.type = EVENT_TYPE_RESET;
	ctx->handler->run(ctx->thread_data_ctx[id].handle, &e);

	/* reset all other handlers */
	e.type = EVENT_TYPE_RESET_ASYNC;

	for (i = 0; i < ctx->pipeline_count; i++) {
		if (i == id)
			continue;

		os_mq_send(&ctx->thread_data_ctx[i].mqueue, &e, 0, 0);
	}

	/* wait for asynchronous reset execution */
	for (i = 0; i < ctx->pipeline_count; i++) {
		if (i == id)
			continue;

		os_sem_take(&ctx->thread_data_ctx[i].async_sem, 0, OS_SEM_TIMEOUT_MAX);
	}

	e.type = EVENT_TYPE_DATA;
	if (ctx->handler->run(ctx->thread_data_ctx[id].handle, &e) < 0)
		os_assert(false, "handler couldn't restart");

	if (id == 0) {
#if USE_TX_IRQ
		sai_enable_irq(&ctx->dev[SAI_DEV_IRQ_SOURCE], false, true);
#else
		sai_enable_irq(&ctx->dev[SAI_DEV_IRQ_SOURCE], true, false);
#endif
	}

	/* restart other handlers */
	for (i = 0; i < ctx->pipeline_count; i++) {
		if (i == id)
			continue;

		os_mq_send(&ctx->thread_data_ctx[i].mqueue, &e, 0, 0);
	}
	os_sem_give(&ctx->reset_sem, 0);
}

void audio_process_data(void *context, uint8_t thread_id)
{
	struct data_ctx *ctx = context;
	struct event e;

	if (!os_mq_receive(&ctx->thread_data_ctx[thread_id].mqueue, &e, 0, OS_QUEUE_EVENT_TIMEOUT_MAX)) {

		os_sem_take(&ctx->thread_data_ctx[thread_id].semaphore, 0, OS_SEM_TIMEOUT_MAX);

		if (ctx->handler) {
			if (ctx->handler->run(ctx->thread_data_ctx[thread_id].handle, &e) != 0) {
				audio_reset(ctx, thread_id);
			} else {
				if (thread_id == 0 && e.type == EVENT_TYPE_DATA) {
#if USE_TX_IRQ
					sai_enable_irq(&ctx->dev[SAI_DEV_IRQ_SOURCE], false, true);
#else
					sai_enable_irq(&ctx->dev[SAI_DEV_IRQ_SOURCE], true, false);
#endif
				}
			}

		}
		os_sem_give(&ctx->thread_data_ctx[thread_id].semaphore, 0);
	}
}

static void audio_stats(struct data_ctx *ctx)
{
	int i;

	if (ctx->handler) {
		for (i = 0; i < ctx->pipeline_count; i++)
			ctx->handler->stats(ctx->thread_data_ctx[i].handle);

		log_info("callback: %llu\n", ctx->callback);
	}
}

static void response(struct rpmsg_ept *ept, uint32_t status)
{
	struct hrpn_resp_audio resp;

	resp.type = HRPN_RESP_TYPE_AUDIO;
	resp.status = status;
	rpmsg_send(ept, &resp, sizeof(resp));
}

static int audio_run(struct data_ctx *ctx, struct hrpn_cmd_audio_run *run)
{
	int rc = HRPN_RESP_STATUS_ERROR;
	struct audio_config cfg;
	struct play_pipeline_config *play_cfg;
	struct event e;
	uint8_t pipeline_count = 0;
	size_t period = DEFAULT_PERIOD;
	uint32_t rate = DEFAULT_SAMPLE_RATE;
	int i;

	if (ctx->handler)
		goto exit;

	if (run->id >= ARRAY_SIZE(g_handler) || !g_handler[run->id].init)
		goto exit;

	play_cfg = g_handler[run->id].data;
	/* Check the count of pipeline */
	for (i = 0; i < ctx->thread_count; i++) {
		if (play_cfg->cfg[i] == NULL)
			break;
		else
			pipeline_count++;
	}
	os_assert(pipeline_count > 0, "At lease one pipeline should be provided.");

	audio_set_hw_addr(&cfg, run->addr);

#if (CONFIG_GENAVB_ENABLE == 1)
	if (system_config_set_net(0, cfg.address)) {
		log_warn("system_config_set_net() failed\n");
	}
#endif

	if (assign_nonzero_valid_val(period, run->period, supported_period) != 0) {
		log_err("Period %d frames is not supported\n", run->period);
		goto exit;
	}

	if (assign_nonzero_valid_val(rate, run->frequency, supported_rate) != 0) {
		log_err("Rate %d Hz is not supported\n", run->frequency);
		goto exit;
	}

	ctx->callback = 0;
	ctx->sample_rate = rate;
	ctx->chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	ctx->bit_width = DEMO_AUDIO_BIT_WIDTH;
	ctx->period = period;
	cfg.rate = rate;
	cfg.period = period;

	audio_check_params(&cfg, run->id);

	for (i = 0; i < pipeline_count; i++) {
		cfg.data = (void *)play_cfg->cfg[i];
		cfg.pipeline_id = i;
		cfg.async_sem = &ctx->thread_data_ctx[i].async_sem;
		ctx->thread_data_ctx[i].handle = g_handler[run->id].init(&cfg);
		if (!ctx->thread_data_ctx[i].handle)
			goto exit;
	}

	sai_setup(ctx);

	for (i = 0; i < pipeline_count; i++) {
		os_sem_take(&ctx->thread_data_ctx[i].semaphore, 0, OS_SEM_TIMEOUT_MAX);
	}
	ctx->handler = &g_handler[run->id];
	ctx->pipeline_count = pipeline_count;
	for (i = 0; i < pipeline_count; i++) {
		os_sem_give(&ctx->thread_data_ctx[i].semaphore, 0);
	}

	/* Send an event to trigger data thread processing */
	e.type = EVENT_TYPE_DATA;
	for (i = 0; i < pipeline_count; i++) {
		os_mq_send(&ctx->thread_data_ctx[i].mqueue, &e, 0, 0);
	}

	rc = HRPN_RESP_STATUS_SUCCESS;

exit:
	return rc;
}

static int audio_stop(struct data_ctx *ctx)
{
	const struct mode_handler *handler;
	int i;

	if (!ctx->handler)
		goto exit;

	for (i = 0; i < ctx->pipeline_count; i++) {
		os_sem_take(&ctx->thread_data_ctx[i].semaphore, 0, OS_SEM_TIMEOUT_MAX);
	}
	handler = ctx->handler;
	ctx->handler = NULL;
	for (i = 0; i < ctx->pipeline_count; i++) {
		os_sem_give(&ctx->thread_data_ctx[i].semaphore, 0);
	}

	for (i = 0; i < ctx->pipeline_count; i++) {
		handler->exit(ctx->thread_data_ctx[i].handle);
	}

	sai_close(ctx);

	for (i = 0; i < sai_active_list_nelems; i++)
		codec_close(sai_active_list[i].cid);

exit:
	return HRPN_RESP_STATUS_SUCCESS;
}

static void audio_command_handler(struct data_ctx *ctx)
{
	struct hrpn_command cmd;
	struct rpmsg_ept *ept= ctx->ctrl.ept;
	unsigned int len;
	int rc = 0;

	len = sizeof(cmd);
	if (rpmsg_recv(ept, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_AUDIO_RUN:
		if (len != sizeof(struct hrpn_cmd_audio_run)) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = audio_run(ctx, &cmd.u.audio_run);

		response(ept, rc);

		break;

	case HRPN_CMD_TYPE_AUDIO_STOP:
		if (len != sizeof(struct hrpn_cmd_audio_stop)) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = audio_stop(ctx);

		response(ept, rc);

		break;

	case HRPN_CMD_TYPE_AUDIO_PIPELINE_DUMP:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_DUMP:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT:
		audio_pipeline_ctrl(&cmd.u.audio_pipeline, len, ept);

		break;

	default:
		response(ept, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

#if (CONFIG_GENAVB_ENABLE == 1)
static void audio_control_handler(struct data_ctx *ctx)
{
	if (ctx->handler && ctx->handler->ctrl)
		ctx->handler->ctrl(ctx->thread_data_ctx[0].handle);
}
#endif

#define CONTROL_POLL_PERIOD	100
#define STATS_POLL_PERIOD	10000
#define STATS_COUNT		(STATS_POLL_PERIOD / CONTROL_POLL_PERIOD)

void audio_control_loop(void *context)
{
	struct data_ctx *ctx = context;
	int count;

	count = STATS_COUNT;
	do {
		/* handle commands from Linux ctrl application */
		audio_command_handler(ctx);

#if (CONFIG_GENAVB_ENABLE == 1)
		/* handle events from control channel(s) */
		audio_control_handler(ctx);
#endif

		count--;
		if (!count) {
			audio_stats(ctx);
			os_cpu_load_stats();
			count = STATS_COUNT;
		}

		os_msleep(CONTROL_POLL_PERIOD);

	} while(1);
}

void *audio_control_init(uint8_t thread_count)
{
	struct data_ctx *audio_ctx;
	int i;
	int err = 0;

	audio_ctx = os_malloc(sizeof(*audio_ctx));
	os_assert(audio_ctx, "Audio context failed with memory allocation error");
	memset(audio_ctx, 0, sizeof(*audio_ctx));

	audio_ctx->thread_count = thread_count;

	audio_ctx->ctrl.ept = rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw");
	os_assert(audio_ctx->ctrl.ept, "rpmsg transport initialization failed!");

	for (i = 0; i < thread_count; i++) {
		err = os_sem_init(&audio_ctx->thread_data_ctx[i].semaphore, 1);
		os_assert(!err, "semaphore initialization failed!");

		err = os_sem_init(&audio_ctx->thread_data_ctx[i].async_sem, 0);
		os_assert(!err, "asynchronous semaphone initialization failed!");

		err = os_sem_init(&audio_ctx->reset_sem, 1);
		os_assert(!err, "reset semaphore initialization failed!");

		err = os_mq_open(&audio_ctx->thread_data_ctx[i].mqueue, "audio_mqueue", 10, sizeof(struct event));
		os_assert(!err, "message queue initialization failed!");
	}

	return audio_ctx;
}
