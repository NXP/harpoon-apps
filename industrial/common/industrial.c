/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hlog.h"
#include "os/string.h"

#include "os/cpu_load.h"
#include "hrpn_ctrl.h"

#include "industrial.h"
#include "rpmsg.h"
#include "rtos_abstraction_layer.h"

#define EPT_ADDR	(30)

extern const struct industrial_use_case use_cases[];

void *industrial_get_data_ctx(struct industrial_ctx *ctx, int use_case_id)
{
	return &ctx->data[use_case_id];
}

static void data_send_event(void *userData, uint8_t type, uint8_t data)
{
	rtos_mqueue_t *mqueue = userData;
	bool yield = false;
	struct event e;

	e.type = type;
	e.data = data;

	rtos_mqueue_send_from_isr(mqueue, &e, RTOS_NO_WAIT, &yield);
	rtos_yield_from_isr(yield);
}

static void industrial_process_data(void *context)
{
	struct data_ctx *data = context;
	struct event e;

	if (!rtos_mqueue_receive(data->mqueue_h, &e, RTOS_WAIT_FOREVER)) {

		rtos_mutex_lock(&data->mutex, RTOS_WAIT_FOREVER);

		if (data->ops)
			data->ops->run(data->priv, &e);

		rtos_mutex_unlock(&data->mutex);
	}
}

static void industrial_stats(struct industrial_ctx *ctx)
{
	int i;

	for (i = 0; i < INDUSTRIAL_USE_CASE_MAX; i++) {
		struct data_ctx *data = &ctx->data[i];

		if (data->ops)
			data->ops->stats(data->priv);
	}
}

static void response(struct rpmsg_ept *ept, uint32_t status)
{
	struct hrpn_resp_industrial resp;

	resp.type = HRPN_RESP_TYPE_INDUSTRIAL;
	resp.status = status;
	rpmsg_send(ept, &resp, sizeof(resp));
}

static void industrial_set_hw_addr(struct industrial_config *cfg, uint8_t *hw_addr)
{
	uint8_t *addr = cfg->address;

	memcpy(addr, hw_addr, sizeof(cfg->address));

	log_info("%02x:%02x:%02x:%02x:%02x:%02x\n",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static int industrial_run(struct data_ctx *data, struct hrpn_cmd_industrial_run *on)
{
	int rc = HRPN_RESP_STATUS_ERROR;
	struct industrial_config cfg;
	const struct industrial_use_case *uc = &use_cases[data->id];
	struct event e;

	if (data->ops)
		goto exit;

	if (on->mode >= uc->ops_num)
		goto exit;

	cfg.event_send = data_send_event;
	cfg.event_data = data->mqueue_h;
	cfg.role = on->role;
	cfg.period = on->period;
	cfg.protocol = on->protocol;
	cfg.num_io_devices = on->num_io_devices;
	cfg.control_strategy = on->control_strategy;
	cfg.app_mode = on->app_mode;

	industrial_set_hw_addr(&cfg, on->addr);

	data->priv = uc->ops[on->mode].init(&cfg);
	if (!data->priv)
		goto exit;

	rtos_mutex_lock(&data->mutex, RTOS_WAIT_FOREVER);
	data->ops = &uc->ops[on->mode];
	rtos_mutex_unlock(&data->mutex);

	/* Send an event to trigger data thread processing */
	e.type = EVENT_TYPE_START;
	rtos_mqueue_send(data->mqueue_h, &e, RTOS_NO_WAIT);

	rc = HRPN_RESP_STATUS_SUCCESS;

exit:
	return rc;
}

static int industrial_stop(struct data_ctx *data)
{
	const struct mode_operations *ops;

	if (!data->ops)
		goto exit;

	data->ops->stats(data->priv);

	rtos_mutex_lock(&data->mutex, RTOS_WAIT_FOREVER);
	ops = data->ops;
	data->ops = NULL;
	rtos_mutex_unlock(&data->mutex);

	ops->exit(data->priv);

exit:
	return HRPN_RESP_STATUS_SUCCESS;
}

static void industrial_command_handler(struct industrial_ctx *ctx)
{
	struct hrpn_command cmd;
	struct rpmsg_ept *ept = ctx->ctrl.ept;
	struct data_ctx *data = NULL;
	unsigned int len;
	int rc;

	len = sizeof(cmd);
	if (rpmsg_recv(ept, &cmd, &len) < 0)
		return;

	switch (cmd.u.cmd.type) {
	case HRPN_CMD_TYPE_CAN_RUN:
		if (!data)
			data = industrial_get_data_ctx(ctx, INDUSTRIAL_USE_CASE_CAN);
		/* fallthrough */
	case HRPN_CMD_TYPE_ETHERNET_RUN:
		if (!data)
			data = industrial_get_data_ctx(ctx, INDUSTRIAL_USE_CASE_ETHERNET);
		if (len != sizeof(struct hrpn_cmd_industrial_run)) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		log_debug("data %p: ept=%p type=%x val=%p\n", data, ept, cmd.u.cmd.type, &cmd.u.industrial_run);
		rc = industrial_run(data, &cmd.u.industrial_run);

		response(ept, rc);

		break;

	case HRPN_CMD_TYPE_CAN_STOP:
		if (!data)
			data = industrial_get_data_ctx(ctx, INDUSTRIAL_USE_CASE_CAN);
		/* fallthrough */
	case HRPN_CMD_TYPE_ETHERNET_STOP:
		if (!data)
			data = industrial_get_data_ctx(ctx, INDUSTRIAL_USE_CASE_ETHERNET);
		if (len != sizeof(struct hrpn_cmd_industrial_stop)) {
			response(ept, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = industrial_stop(data);

		response(ept, rc);

		break;

	default:
		response(ept, HRPN_RESP_STATUS_ERROR);
		break;
	}
}

#define CONTROL_POLL_PERIOD	100
#define STATS_POLL_PERIOD	10000
#define STATS_COUNT		(STATS_POLL_PERIOD / CONTROL_POLL_PERIOD)

void industrial_control_loop(void *context)
{
	struct industrial_ctx *ctx = context;
	static int count = STATS_COUNT;

	industrial_command_handler(ctx);

	count--;
	if (!count) {
		industrial_stats(ctx);
		os_cpu_load_stats();
		count = STATS_COUNT;
	}

	rtos_sleep(RTOS_MS_TO_TICKS(CONTROL_POLL_PERIOD));
}

static int data_ctx_init(struct data_ctx *data)
{
	int err;

	err = rtos_mutex_init(&data->mutex);
	rtos_assert(!err, "mutex initialization failed!");

	data->mqueue_h = rtos_mqueue_alloc_init(10, sizeof(struct event));
	rtos_assert(data->mqueue_h, "message queue initialization failed!");

	data->process_data = industrial_process_data;

	return err;
}

void *industrial_control_init(int nb_use_cases)
{
	struct industrial_ctx *ctx;
	int i, err = 0;

	ctx = rtos_malloc(sizeof(*ctx));
	rtos_assert((ctx != NULL), "memory allocation error");

	memset(ctx, 0, sizeof(*ctx));

	ctx->ctrl.ept = rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw");
	rtos_assert(ctx->ctrl.ept, "rpmsg transport initialization failed!");

	for (i = 0; i < nb_use_cases; i++) {
		struct data_ctx *data = &ctx->data[i];

		err = data_ctx_init(data);
		rtos_assert(!err, "industrial data context %d failed!", i);

		data->id = i;
	}

	return ctx;
}
