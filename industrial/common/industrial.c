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
#include "os/string.h"
#include "os/unistd.h"

#include "os/cpu_load.h"
#include "ivshmem.h"
#include "mailbox.h"
#include "hrpn_ctrl.h"

#include "industrial.h"

#ifdef MBOX_TRANSPORT_RPMSG
#include "rpmsg.h"

#define EPT_ADDR (30)
#endif

extern const struct industrial_use_case use_cases[];

void *industrial_get_data_ctx(struct industrial_ctx *ctx, int use_case_id)
{
	return &ctx->data[use_case_id];
}

static void data_send_event(void *userData, uint8_t status)
{
	os_mqd_t *mqueue = userData;
	struct event e;

	e.type = EVENT_TYPE_TX_RX;
	e.data = status;

	os_mq_send(mqueue, &e, OS_MQUEUE_FLAGS_ISR_CONTEXT, 0);
}

static void industrial_process_data(void *context)
{
	struct data_ctx *data = context;
	struct event e;

	if (!os_mq_receive(&data->mqueue, &e, 0, OS_QUEUE_EVENT_TIMEOUT_MAX)) {

		os_sem_take(&data->semaphore, 0, OS_SEM_TIMEOUT_MAX);

		if (data->ops)
			data->ops->run(data->priv, &e);

		os_sem_give(&data->semaphore, 0);
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

static void response(struct mailbox *mb, uint32_t status)
{
	struct hrpn_resp_industrial resp;

	resp.type = HRPN_RESP_TYPE_INDUSTRIAL;
	resp.status = status;
	mailbox_resp_send(mb, &resp, sizeof(resp));
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

	if (on->mode >= ARRAY_SIZE(uc->ops))
		goto exit;

	cfg.event_send = data_send_event;
	cfg.event_data = &data->mqueue;
	cfg.role = on->role;

	industrial_set_hw_addr(&cfg, on->addr);

	data->priv = uc->ops[on->mode].init(&cfg);
	if (!data->priv)
		goto exit;

	os_sem_take(&data->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	data->ops = &uc->ops[on->mode];
	os_sem_give(&data->semaphore, 0);

	/* Send an event to trigger data thread processing */
	e.type = EVENT_TYPE_START;
	os_mq_send(&data->mqueue, &e, 0, 0);

	rc = HRPN_RESP_STATUS_SUCCESS;

exit:
	return rc;
}

static int industrial_stop(struct data_ctx *data)
{
	const struct mode_operations *ops;

	if (!data->ops)
		goto exit;

	os_sem_take(&data->semaphore, 0, OS_SEM_TIMEOUT_MAX);
	ops = data->ops;
	data->ops = NULL;
	os_sem_give(&data->semaphore, 0);

	ops->exit(data->priv);

exit:
	return HRPN_RESP_STATUS_SUCCESS;
}

static void industrial_command_handler(struct industrial_ctx *ctx)
{
	struct hrpn_command cmd;
	struct mailbox *mb = &ctx->ctrl.mb;
	struct data_ctx *data = NULL;
	unsigned int len;
	int rc;

	len = sizeof(cmd);
	if (mailbox_cmd_recv(mb, &cmd, &len) < 0)
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
			response(mb, HRPN_RESP_STATUS_ERROR);
			break;
		}

		log_debug("data %p: mb=%p type=%x val=%p\n", data, mb, cmd.u.cmd.type, &cmd.u.industrial_run);
		rc = industrial_run(data, &cmd.u.industrial_run);

		response(mb, rc);

		break;

	case HRPN_CMD_TYPE_CAN_STOP:
		if (!data)
			data = industrial_get_data_ctx(ctx, INDUSTRIAL_USE_CASE_CAN);
		/* fallthrough */
	case HRPN_CMD_TYPE_ETHERNET_STOP:
		if (!data)
			data = industrial_get_data_ctx(ctx, INDUSTRIAL_USE_CASE_ETHERNET);
		if (len != sizeof(struct hrpn_cmd_industrial_stop)) {
			response(mb, HRPN_RESP_STATUS_ERROR);
			break;
		}

		rc = industrial_stop(data);

		response(mb, rc);

		break;

	default:
		response(mb, HRPN_RESP_STATUS_ERROR);
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

	os_msleep(CONTROL_POLL_PERIOD);
}

static int data_ctx_init(struct data_ctx *data)
{
	int err;

	err = os_sem_init(&data->semaphore, 1);
	os_assert(!err, "semaphore initialization failed!");

	err = os_mq_open(&data->mqueue, "industrial_mqueue", 10, sizeof(struct event));
	os_assert(!err, "message queue initialization failed!");

	data->process_data = industrial_process_data;

	return err;
}

#ifdef MBOX_TRANSPORT_RPMSG
static int rpmsg_transport_init(int link_id, int ept_addr, const char *sn,
				void **tp, void **cmd, void **resp)
{
	struct rpmsg_instance *ri;
	struct rpmsg_ept *ept;

	ri = rpmsg_init(link_id);
	os_assert(ri, "rpmsg initialization failed, cannot proceed\n");
	ept = rpmsg_create_ept(ri, ept_addr, sn);
	os_assert(ept, "rpmsg ept creation failed, cannot proceed\n");
	*tp = ept;
	*cmd = os_malloc(1024);
	os_assert(*cmd, "malloc mailbox memory faild, cannot proceed\n");
	*resp = *cmd + 512;
	memset(*cmd, 0, 1024);

	return 0;
}

#else

static int ivshmem_transport_init(unsigned int bdf, struct ivshmem *mem,
				  void **tp, void **cmd, void **resp)
{
	int rc;

	if (!mem) {
		mem = os_malloc(sizeof(*mem));
		os_assert(mem, "malloc for ivshmem struct faild, cannot proceed\n");
	}

	rc = ivshmem_init(bdf, mem);
	os_assert(!rc, "ivshmem initialization failed, can not proceed\n");

	os_assert(mem->out_size, "ivshmem mis-configuration, can not proceed\n");

	*cmd = mem->out[0];
	*resp = mem->out[mem->id];
	*tp = NULL;

	return 0;
}
#endif

static int ctrl_ctx_init(struct ctrl_ctx *ctrl)
{
	int err;
	void *tp = NULL;
	void *cmd, *resp;

#ifdef MBOX_TRANSPORT_RPMSG
	err = rpmsg_transport_init(RL_BOARD_RPMSG_LINK_ID, EPT_ADDR, "rpmsg-raw",
				   &tp, &cmd, &resp);
	os_assert(!err, "rpmsg transport initialization failed, cannot proceed\n");
#else /* IVSHMEM */
	err = ivshmem_transport_init(0, &ctrl->mem, &tp, &cmd, &resp);
	os_assert(!err, "ivshmem transport initialization failed, cannot proceed\n");
#endif
	err = mailbox_init(&ctrl->mb, cmd, resp, false, tp);
	os_assert(!err, "mailbox initialization failed!");

	return err;
}

void *industrial_control_init(int nb_use_cases)
{
	struct industrial_ctx *ctx;
	int i, err;

	ctx = os_malloc(sizeof(*ctx));
	os_assert((ctx != NULL), "memory allocation error");

	memset(ctx, 0, sizeof(*ctx));

	err = ctrl_ctx_init(&ctx->ctrl);
	os_assert(!err, "industrial ctrl context failed!");

	for (i = 0; i < nb_use_cases; i++) {
		struct data_ctx *data = &ctx->data[i];

		err = data_ctx_init(data);
		os_assert(!err, "industrial data context %d failed!", i);

		data->id = i;
	}

	return ctx;
}
