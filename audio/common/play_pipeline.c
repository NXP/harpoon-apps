/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/assert.h"
#include "os/semaphore.h"
#include "os/stdlib.h"

#include "app_board.h"

#include "audio_pipeline.h"
#include "audio.h"
#include "hlog.h"
#include "hrpn_ctrl.h"

#if (CONFIG_GENAVB_ENABLE == 1)
#include "avb_hardware.h"
#include "avb_tsn/clock_domain.h"
#include "avb_tsn/genavb.h"
#include "avb_tsn/stats_task.h"
#include "genavb/genavb.h"
#include "os/irq.h"
#include "system_config.h"

extern void BOARD_NET_PORT0_DRV_IRQ0_HND(void);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
extern void BOARD_NET_PORT0_DRV_IRQ1_HND(void);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
extern void BOARD_NET_PORT0_DRV_IRQ2_HND(void);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
extern void BOARD_NET_PORT0_DRV_IRQ3_HND(void);
#endif

extern void BOARD_GENAVB_TIMER_0_IRQ_HANDLER(void);

#define STATS_PERIOD_MS 2000

struct avtp_avb_ctx {
	struct genavb_control_handle *ctrl_h;
	struct genavb_control_handle *controlled_h;
	unsigned int aem_id;
	bool milan_mode;
};

#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

struct pipeline_ctx {
	os_sem_t *async_sem;
	struct audio_pipeline *pipeline;

	struct {
		uint64_t run;
		uint64_t err;
	} stats;

#if (CONFIG_GENAVB_ENABLE == 1)
	struct avtp_avb_ctx avb;
#endif

	/* pipeline index, '0' for master pipeline */
	uint8_t id;
};

void play_pipeline_stats(void *handle)
{
	struct pipeline_ctx *ctx = handle;

	log_info("pipeline%d  run: %llu, err: %llu\n", ctx->id,
			ctx->stats.run, ctx->stats.err);

	audio_pipeline_stats(ctx->pipeline);
}

int play_pipeline_run(void *handle, struct event *e)
{
	struct pipeline_ctx *ctx = handle;
	int err = 0;

	switch (e->type) {
	case EVENT_TYPE_DATA:
		ctx->stats.run++;

		err = audio_pipeline_run(ctx->pipeline);
		if (err) {
			ctx->stats.err++;
			break;
		}
		break;

	case EVENT_TYPE_RESET:
		audio_pipeline_reset(ctx->pipeline);
		break;

	case EVENT_TYPE_RESET_ASYNC:
		audio_pipeline_reset(ctx->pipeline);
		os_sem_give(ctx->async_sem, 0);
		break;

	default:
		break;
	}

	return err;
}

#if (CONFIG_GENAVB_ENABLE == 1)

static void listener_disconnect(unsigned int stream_index)
{
	struct hrpn_cmd_audio_element_avtp_disconnect disconnect;
	int i;

	/* need to disconnect streams in AVTP audio element */
	disconnect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_DISCONNECT;
	disconnect.pipeline.id = 0;
	disconnect.element.type = AUDIO_ELEMENT_AVTP_SOURCE;
	disconnect.element.id = 0;
	disconnect.stream_index = stream_index;

	for (i = 0; i < MAX_PIPELINES; i++) {
		disconnect.pipeline.id = i;
		audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&disconnect, sizeof(disconnect), NULL);
	}
}

static void listener_connect(struct genavb_msg_media_stack_connect *media_stack_connect)
{
	struct hrpn_cmd_audio_element_avtp_connect connect;
	int i;

	/* need to connect streams in AVTP audio element */
	connect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_CONNECT;
	connect.pipeline.id = 0;
	connect.element.type = AUDIO_ELEMENT_AVTP_SOURCE;
	connect.element.id = 0;
	connect.stream_index = media_stack_connect->stream_index;
	connect.stream_params = media_stack_connect->stream_params;

	for (i = 0; i < MAX_PIPELINES; i++) {
		connect.pipeline.id = i;
		audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&connect, sizeof(connect), NULL);
	}
}

static void talker_disconnect(unsigned int stream_index)
{
	struct hrpn_cmd_audio_element_avtp_disconnect disconnect;
	int i;

	/* need to disconnect streams in AVTP audio element */
	disconnect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SINK_DISCONNECT;
	disconnect.pipeline.id = 0;
	disconnect.element.type = AUDIO_ELEMENT_AVTP_SINK;
	disconnect.element.id = 0;
	disconnect.stream_index = stream_index;

	for (i = 0; i < MAX_PIPELINES; i++) {
		disconnect.pipeline.id = i;
		audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&disconnect, sizeof(disconnect), NULL);
	}
}

static void talker_connect(struct genavb_msg_media_stack_connect *media_stack_connect)
{
	struct hrpn_cmd_audio_element_avtp_connect connect;
	int i;

	/* need to connect streams in AVTP audio element */
	connect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SINK_CONNECT;
	connect.pipeline.id = 0;
	connect.element.type = AUDIO_ELEMENT_AVTP_SINK;
	connect.element.id = 0;
	connect.stream_index = media_stack_connect->stream_index;
	connect.stream_params = media_stack_connect->stream_params;

	for (i = 0; i < MAX_PIPELINES; i++) {
		connect.pipeline.id = i;
		audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&connect, sizeof(connect), NULL);
	}
}

static void handle_avdecc_event(struct pipeline_ctx *ctx, struct genavb_control_handle *ctrl_h)
{
	struct genavb_msg_media_stack_connect *media_stack_connect;
	struct genavb_msg_media_stack_disconnect *media_stack_disconnect;
	union genavb_media_stack_msg msg;
	genavb_msg_type_t msg_type;
	unsigned int msg_len;
	int rc;

	msg_len = sizeof(union genavb_media_stack_msg);

	rc = genavb_control_receive(ctrl_h, &msg_type, &msg, &msg_len);
	if (rc != GENAVB_SUCCESS) {
		/* no event message*/

		goto exit;
	}

	switch (msg_type) {
	case GENAVB_MSG_MEDIA_STACK_CONNECT:

		media_stack_connect = &msg.media_stack_connect;

		log_info("GENAVB_MSG_MEDIA_STACK_CONNECT stream index: %u\n",
				media_stack_connect->stream_index);

		if (media_stack_connect->stream_params.direction == AVTP_DIRECTION_LISTENER)
			listener_connect(media_stack_connect);
		else
			talker_connect(media_stack_connect);

		break;

	case GENAVB_MSG_MEDIA_STACK_DISCONNECT:

		media_stack_disconnect = &msg.media_stack_disconnect;

		log_info("GENAVB_MSG_MEDIA_STACK_DISCONNECT stream index: %u\n",
			media_stack_disconnect->stream_index);

		if (media_stack_disconnect->direction == AVTP_DIRECTION_LISTENER)
			listener_disconnect(media_stack_disconnect->stream_index);
		else
			talker_disconnect(media_stack_disconnect->stream_index);

		break;

	default:
		log_err("Error, unknown message type: %d\n", msg_type);
		rc = -1;
		break;
	}

exit:
	return;
}

static void handle_avdecc_controlled_event(struct pipeline_ctx *ctx, struct genavb_control_handle *controlled_h)
{
	unsigned short status = AECP_AEM_SUCCESS;
	union genavb_controlled_msg msg;
	struct aecp_aem_pdu *pdu;
	genavb_msg_type_t msg_type;
	uint16_t cmd_type;
	unsigned int msg_len;
	int rc;

	msg_len = sizeof(union genavb_controlled_msg);

	rc = genavb_control_receive(controlled_h, &msg_type, &msg, &msg_len);
	if (rc != GENAVB_SUCCESS) {
		/* no event messages */

		goto exit;
	}

	switch (msg_type) {
	case GENAVB_MSG_AECP:
		pdu = (struct aecp_aem_pdu *)msg.aecp.buf;

		cmd_type = AECP_AEM_GET_CMD_TYPE(pdu);
		log_info("AECP command type (0x%x) seq_id (%d)\n", cmd_type, ntohs(pdu->sequence_id));

		switch (cmd_type) {
		case AECP_AEM_CMD_GET_AUDIO_MAP:
		{
			/* GET_AUDIO_MAP not fully supported, simply respond with empty audio mappings */
			struct aecp_aem_get_audio_map_rsp_pdu *audio_map_rsp  = (struct aecp_aem_get_audio_map_rsp_pdu *)(pdu + 1);

			audio_map_rsp->number_of_maps = htons(0);
			audio_map_rsp->number_of_mappings = htons(0);
			audio_map_rsp->reserved = htons(0);

			/* Set the AECP Response PDU length to match the aecp_aem_get_audio_map_rsp_pdu to be sent back */
			msg.aecp.len = sizeof(struct aecp_aem_pdu) + sizeof(struct aecp_aem_get_audio_map_rsp_pdu);

			break;
		}
		default:
			log_info("AECP command type (0x%x) not handled in this app, skip\n", cmd_type);
			status = AECP_AEM_NOT_IMPLEMENTED;

			break;
		}

		msg.aecp.msg_type = AECP_AEM_RESPONSE;
		msg.aecp.status = status;

		rc = genavb_control_send(controlled_h, msg_type, &msg, msg_len);
		if (rc != GENAVB_SUCCESS) {
			log_info("AECP command response send failed: %d(%s)\n", rc, genavb_strerror(rc));
		}
		break;
	default:
		log_warn("Unsupported AVDECC message type (%d).\n", msg_type);
		break;
	}

exit:
	return;
}
void play_pipeline_ctrl_avb(void *handle)
{
	struct pipeline_ctx *ctx = handle;

	handle_avdecc_event(ctx, ctx->avb.ctrl_h);
	handle_avdecc_controlled_event(ctx, ctx->avb.controlled_h);
}

static int avb_setup(struct pipeline_ctx *ctx)
{
	int genavb_result;
	int rc;

	log_info("enter\n");

	avb_hardware_init();

	os_irq_register(BOARD_NET_PORT0_DRV_IRQ0, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ0_HND, NULL, OS_IRQ_PRIO_DEFAULT);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ1, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ1_HND, NULL, OS_IRQ_PRIO_DEFAULT);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ2, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ2_HND, NULL, OS_IRQ_PRIO_DEFAULT);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ3, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ3_HND, NULL, OS_IRQ_PRIO_DEFAULT);
#endif

	os_irq_register(BOARD_GENAVB_TIMER_0_IRQ, (void (*)(void(*)))BOARD_GENAVB_TIMER_0_IRQ_HANDLER, NULL, OS_IRQ_PRIO_DEFAULT);

#if (CONFIG_GENAVB_USE_AVDECC == 1)
	system_config_set_avdecc(ctx->avb.aem_id, ctx->avb.milan_mode);
#endif

	rc = gavb_stack_init();
	if (rc) {
		log_err("gavb_stack_init() failed\n");
		goto exit;
	}

	genavb_result = genavb_clock_domain_init(get_genavb_handle());
	if (genavb_result != GENAVB_SUCCESS) {
		log_err("clock_domain_init() failed\n");

		rc = -1;
		goto exit;
	}

	/* open avdecc control channel */
	genavb_result = genavb_control_open(get_genavb_handle(), &ctx->avb.ctrl_h, GENAVB_CTRL_AVDECC_MEDIA_STACK);
	if (genavb_result != GENAVB_SUCCESS) {
		log_err("genavb_control_open(GENAVB_CTRL_AVDECC_MEDIA_STACK) failed: %s\n", genavb_strerror(genavb_result));
		rc = -1;

		goto exit;
	}
	/*
	* Open controlled channel for AVDECC commands.
	*/
	genavb_result = genavb_control_open(get_genavb_handle(), &ctx->avb.controlled_h, GENAVB_CTRL_AVDECC_CONTROLLED);
	if (genavb_result != GENAVB_SUCCESS) {
		log_err("genavb_control_open(GENAVB_CTRL_AVDECC_CONTROLLED) failed: %s\n", genavb_strerror(rc));
		rc = -1;

		goto exit;
	}

	if (STATS_TaskInit(NULL, NULL, STATS_PERIOD_MS) < 0)
		log_err("STATS_TaskInit() failed\n");

exit:
	return rc;
}

static void avb_close(struct pipeline_ctx *ctx)
{
	genavb_control_close(ctx->avb.ctrl_h);

	if (gavb_stack_exit()) {
		log_err("gavb_stack_exit() failed\n");
	}

	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ0);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ1);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ2);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
	os_irq_unregister(BOARD_NET_PORT0_DRV_IRQ3);
#endif

	os_irq_unregister(BOARD_GENAVB_TIMER_0_IRQ);

	STATS_TaskExit();

	avb_hardware_exit();
}

void *play_pipeline_init_avb(void *parameters)
{
	struct pipeline_ctx *ctx;
	int rc;

	ctx = play_pipeline_init(parameters);

	if (ctx) {
		/* Only setup AVB once for first pipeline init */
		ctx->avb.aem_id = ((struct audio_pipeline_config *)((struct audio_config *)parameters)->data)->aem_id;
		ctx->avb.milan_mode = ((struct audio_pipeline_config *)((struct audio_config *)parameters)->data)->milan_mode;
		if (ctx->id == 0) {
			rc = avb_setup(ctx);
			if (rc) {
				play_pipeline_exit(ctx);
				ctx = NULL;
			}
		}
	}

	return ctx;
}

void play_pipeline_exit_avb(void *handle)
{
	struct pipeline_ctx *ctx = handle;

	if (ctx->id == 0)
		avb_close(ctx);

	play_pipeline_exit(handle);
}
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

void *play_pipeline_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	struct audio_pipeline_config *pipeline_cfg;
	struct pipeline_ctx *ctx;

	ctx = os_malloc(sizeof(struct pipeline_ctx) + sizeof(struct audio_pipeline_config));
	os_assert(ctx, "Audio pipeline failed with memory allocation error");
	memset(ctx, 0, sizeof(struct pipeline_ctx));

	pipeline_cfg = (struct audio_pipeline_config *)(ctx + 1);

	memcpy(pipeline_cfg, cfg->data, sizeof(struct audio_pipeline_config));

	/* override pipeline configuration */
	pipeline_cfg->sample_rate = cfg->rate;
	pipeline_cfg->period = cfg->period;
	pipeline_cfg->id = cfg->pipeline_id;

	ctx->id = cfg->pipeline_id;
	ctx->pipeline = audio_pipeline_init(pipeline_cfg);
	if (!ctx->pipeline)
		goto err_init;

	ctx->async_sem = cfg->async_sem;

	log_info("Starting %s (Sample Rate: %d Hz, Period: %u frames)\n",
			pipeline_cfg->name, pipeline_cfg->sample_rate, (uint32_t)pipeline_cfg->period);

	return ctx;

err_init:
	os_free(ctx);
	return NULL;
}

void play_pipeline_exit(void *handle)
{
	struct pipeline_ctx *ctx = handle;

	audio_pipeline_exit(ctx->pipeline);

	os_free(ctx);

	log_info("\nEnd.\n");
}
