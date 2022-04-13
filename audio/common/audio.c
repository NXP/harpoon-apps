/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "log.h"
#include "os/assert.h"
#include "os/mqueue.h"
#include "os/semaphore.h"
#include "os/stdlib.h"
#include "os/unistd.h"

#include "idle.h"
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
	int (*run)(void *, struct event *e);
};

struct data_ctx {
	struct ivshmem mem;
	struct mailbox mb;

	os_sem_t semaphore;
	os_mqd_t mqueue;

	const struct mode_handler *handler;
	void *handle;
};

const static struct mode_handler handler[] =
{
	[0] = {
		.init = play_dtmf_init,
		.exit = play_dtmf_exit,
		.run = play_dtmf_run,
		.stats = play_dtmf_stats,
	},
	[1] = {
		.init = play_music_init,
		.exit = play_music_exit,
		.run = play_music_run,
		.stats = play_music_stats,
	},
	[2] = {
		.init = play_sine_init,
		.exit = play_sine_exit,
		.run = play_sine_run,
		.stats = play_sine_stats,
	},
	[3] = {
		.init = rec_play_init,
		.exit = rec_play_exit,
		.run = rec_play_run,
		.stats = rec_play_stats,
	},
	[4] = {
		.init = rec_play2_init,
		.exit = rec_play2_exit,
		.run = rec_play2_run,
		.stats = rec_play2_stats,
	},
	[5] = {
		.init = play_pipeline_init,
		.exit = play_pipeline_exit,
		.run = play_pipeline_run,
		.stats = play_pipeline_stats,
	}
};

static void data_send_event(void *userData, uint8_t status)
{
	os_mqd_t *mqueue = userData;
	struct event e;

	e.type = EVENT_TYPE_TX_RX;
	e.data = status;

	os_mq_send(mqueue, &e, OS_MQUEUE_FLAGS_ISR_CONTEXT, 0);
}

void audio_process_data(void *context)
{
	struct data_ctx *ctx = context;
	struct event e;

	if (!os_mq_receive(&ctx->mqueue, &e, 0, OS_QUEUE_EVENT_TIMEOUT_MAX)) {

		os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);

		if (ctx->handler)
			ctx->handler->run(ctx->handle, &e);

		os_sem_give(&ctx->semaphore, 0);
	}
}

static void audio_stats(struct data_ctx *ctx)
{
	if (ctx->handler)
		ctx->handler->stats(ctx->handle);
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
	struct event e;

	if (ctx->handler)
		goto exit;

	if (run->id >= ARRAY_SIZE(handler))
		goto exit;

	cfg.event_send = data_send_event;
	cfg.event_data = &ctx->mqueue;
	cfg.rate = run->frequency;
	cfg.period = run->period;

	ctx->handle = handler[run->id].init(&cfg);
	if (!ctx->handle)
		goto exit;

	os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	ctx->handler = &handler[run->id];
	os_sem_give(&ctx->semaphore, 0);

	/* Send an event to trigger data thread processing */
	e.type = EVENT_TYPE_START;
	os_mq_send(&ctx->mqueue, &e, 0, 0);

	rc = HRPN_RESP_STATUS_SUCCESS;

exit:
	return rc;
}

static int audio_stop(struct data_ctx *ctx)
{
	const struct mode_handler *handler;

	if (!ctx->handler)
		goto exit;

	os_sem_take(&ctx->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	handler = ctx->handler;
	ctx->handler = NULL;
	os_sem_give(&ctx->semaphore, 0);

	handler->exit(ctx->handle);

exit:
	return HRPN_RESP_STATUS_SUCCESS;
}

static void audio_command_handler(struct data_ctx *ctx)
{
	struct hrpn_command cmd;
	struct mailbox *m = &ctx->mb;;
	unsigned int len;
	int rc;

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

#define CONTROL_POLL_PERIOD	100
#define STATS_POLL_PERIOD	10000
#define STATS_COUNT		(STATS_POLL_PERIOD / CONTROL_POLL_PERIOD)

void audio_control_loop(void *context)
{
	struct data_ctx *ctx = context;
	int count;

	count = STATS_COUNT;
	do {
		audio_command_handler(ctx);

		count--;
		if (!count) {
			audio_stats(ctx);
			cpu_load_stats();
			count = STATS_COUNT;
		}

		os_msleep(CONTROL_POLL_PERIOD);

	} while(1);
}

void *audio_control_init(void)
{
	int err;
	struct data_ctx *audio_ctx;
	struct ivshmem *mem;

	audio_ctx = os_malloc(sizeof(*audio_ctx));
	os_assert(audio_ctx, "Audio context failed with memory allocation error");
	memset(audio_ctx, 0, sizeof(*audio_ctx));

	mem = &audio_ctx->mem;
	err = ivshmem_init(0, mem);
	os_assert(!err, "ivshmem initialization failed, cannot proceed\n");
	os_assert(mem->out_size, "ivshmem mis-configuration, cannot proceed\n");

	mailbox_init(&audio_ctx->mb, mem->out[0], mem->out[mem->id], false);
	os_assert(!err, "mailbox initialization failed!");

	err = os_sem_init(&audio_ctx->semaphore, 1);
	os_assert(!err, "semaphore initialization failed!");

	err = os_mq_open(&audio_ctx->mqueue, "audio_mqueue", 10, sizeof(struct event));
	os_assert(!err, "message queue initialization failed!");

	return audio_ctx;
}
