/*
 * Copyright 2022 NXP
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

#include "ivshmem.h"
#include "mailbox.h"
#include "hrpn_ctrl.h"

#include "audio.h"
#include "audio_entry.h"

#include "audio_pipeline.h"

struct mode_handler {
	void *(*init)(void *);
	void (*exit)(void *);
	void (*stats)(void *);
	void (*ctrl)(void *);
	int (*run)(void *, struct event *e);
	void *data;
};

struct data_ctx {
	struct ivshmem mem;
	struct mailbox mb;
	uint8_t thread_count;
	uint8_t pipeline_count;

	/* The first thread is used for parent pipeline, others are for child pipeline */
	struct thread_data_ctx_t {
		os_sem_t semaphore;
		os_sem_t async_sem;
		os_mqd_t mqueue;
		/* pipeline_ctx handle for current thread */
		void *handle;
	} thread_data_ctx[MAX_AUDIO_DATA_THREADS];

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

static struct play_pipeline_config play_pipeline_smp_config = {
	.cfg = {
		&pipeline_full_thread_0_config,
		&pipeline_full_thread_1_config,
	}
};

#if (CONFIG_GENAVB_ENABLE == 1)
static struct play_pipeline_config play_pipeline_full_avb_config = {
	.cfg = &pipeline_full_avb_config,
};
#endif

const static struct mode_handler handler[] =
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
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.ctrl = play_pipeline_ctrl,
		.data = &play_pipeline_full_avb_config,
	}
#endif
	[5] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
		.data = &play_pipeline_smp_config,
	}

};

static void data_send_event(void *userData, uint8_t status)
{
	struct data_ctx *ctx = userData;
	struct event e;
	int i;

	e.type = EVENT_TYPE_DATA;
	e.data = status;

	for (i = 0; i < ctx->pipeline_count; i++)
		os_mq_send(&ctx->thread_data_ctx[i].mqueue, &e, OS_MQUEUE_FLAGS_ISR_CONTEXT, 0);
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

		if (ctx->handler)
			if (ctx->handler->run(ctx->thread_data_ctx[thread_id].handle, &e) != 0)
				audio_reset(ctx, thread_id);

		os_sem_give(&ctx->thread_data_ctx[thread_id].semaphore, 0);
	}
}

static void audio_stats(struct data_ctx *ctx)
{
	int i;

	if (ctx->handler) {
		for (i = 0; i < ctx->pipeline_count; i++)
			ctx->handler->stats(ctx->thread_data_ctx[i].handle);
	}
}

static void response(struct mailbox *m, uint32_t status)
{
	struct hrpn_resp_audio resp;

	resp.type = HRPN_RESP_TYPE_AUDIO;
	resp.status = status;
	mailbox_resp_send(m, &resp, sizeof(resp));
}

static int audio_run(struct data_ctx *ctx, struct hrpn_cmd_audio_run *run)
{
	int rc = HRPN_RESP_STATUS_ERROR;
	struct audio_config cfg;
	struct play_pipeline_config *play_cfg;
	struct event e;
	uint8_t pipeline_count = 0;
	int i;

	if (ctx->handler)
		goto exit;

	if (run->id >= ARRAY_SIZE(handler) || !handler[run->id].init)
		goto exit;

	play_cfg = handler[run->id].data;
	/* Check the count of pipleline */
	for (i = 0; i < ctx->thread_count; i++) {
		if (play_cfg->cfg[i] == NULL)
			break;
		else
			pipeline_count++;
	}
	os_assert(pipeline_count > 0, "At lease one pipleline should be provided.");

	cfg.event_send = data_send_event;
	cfg.rate = run->frequency;
	cfg.period = run->period;

	for (i = 0; i < pipeline_count; i++) {
		cfg.event_data = ctx;
		cfg.data = (void *)play_cfg->cfg[i];
		cfg.pipeline_id = i;
		cfg.async_sem = &ctx->thread_data_ctx[i].async_sem;
		ctx->thread_data_ctx[i].handle = handler[run->id].init(&cfg);
		if (!ctx->thread_data_ctx[i].handle)
			goto exit;
	}

	for (i = 0; i < pipeline_count; i++) {
		os_sem_take(&ctx->thread_data_ctx[i].semaphore, 0, OS_SEM_TIMEOUT_MAX);
	}
	ctx->handler = &handler[run->id];
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

exit:
	return HRPN_RESP_STATUS_SUCCESS;
}

static void audio_command_handler(struct data_ctx *ctx)
{
	struct hrpn_command cmd;
	struct mailbox *m = &ctx->mb;;
	unsigned int len;
	int rc = 0;

	len = sizeof(cmd);
	if (mailbox_cmd_recv(m, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_AUDIO_RUN:
		if (len != sizeof(struct hrpn_cmd_audio_run)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = audio_run(ctx, &cmd.u.audio_run);

		response(m, rc);

		break;

	case HRPN_CMD_TYPE_AUDIO_STOP:
		if (len != sizeof(struct hrpn_cmd_audio_stop)) {
			response(m, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = audio_stop(ctx);

		response(m, rc);

		break;

	case HRPN_CMD_TYPE_AUDIO_PIPELINE_DUMP:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_DUMP:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_CONNECT:
	case HRPN_CMD_TYPE_AUDIO_ELEMENT_ROUTING_DISCONNECT:
		audio_pipeline_ctrl(&cmd.u.audio_pipeline, len, m);

		break;

	default:
		response(m, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

static void audio_control_handler(struct data_ctx *ctx)
{
	if (ctx->handler && ctx->handler->ctrl)
		ctx->handler->ctrl(ctx->handle);
}


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

		/* handle events from control channel(s) */
		audio_control_handler(ctx);

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
	int err, i;
	struct data_ctx *audio_ctx;
	struct ivshmem *mem;

	audio_ctx = os_malloc(sizeof(*audio_ctx));
	os_assert(audio_ctx, "Audio context failed with memory allocation error");
	memset(audio_ctx, 0, sizeof(*audio_ctx));

	audio_ctx->thread_count = thread_count;

	mem = &audio_ctx->mem;
	err = ivshmem_init(0, mem);
	os_assert(!err, "ivshmem initialization failed, cannot proceed\n");
	os_assert(mem->out_size, "ivshmem mis-configuration, cannot proceed\n");

	mailbox_init(&audio_ctx->mb, mem->out[0], mem->out[mem->id], false);
	os_assert(!err, "mailbox initialization failed!");

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
