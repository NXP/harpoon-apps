/*
 * Copyright 2022 NXP
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
#include "codec_config.h"
#include "sai_clock_config.h"
#include "sai_drv.h"
#include "sai_config.h"

#if (CONFIG_GENAVB_ENABLE == 1)
#include "avb_hardware.h"
#include "avb_tsn/genavb.h"
#include "avb_tsn/stats_task.h"
#include "genavb/genavb.h"
#include "os/irq.h"

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

extern void BOARD_GPT_0_IRQ_HANDLER(void);
extern void BOARD_GPT_1_IRQ_HANDLER(void);

#define STATS_PERIOD_MS 2000

struct avtp_avb_ctx {
	struct genavb_control_handle *ctrl_h;
};

#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

#define DEFAULT_PERIOD		8
#define DEFAULT_SAMPLE_RATE	48000
#define USE_TX_IRQ		1

static const int supported_period[] = {2, 4, 8, 16, 32};
static const uint32_t supported_rate[] = {44100, 48000, 88200, 96000, 176400, 192000};

struct pipeline_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;
	os_sem_t *async_sem;

	struct sai_device dev[SAI_TX_MAX_INSTANCE];
	struct audio_pipeline *pipeline;
	sai_word_width_t bit_width;
	sai_sample_rate_t sample_rate;
	uint32_t chan_numbers;
	uint8_t period;

	struct {
		uint64_t callback;
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

	log_info("pipeline%d  callback: %llu, run: %llu, err: %llu\n", ctx->id,
			ctx->stats.callback, ctx->stats.run, ctx->stats.err);

	audio_pipeline_stats(ctx->pipeline);
}

static void rx_callback(uint8_t status, void *user_data)
{
	struct pipeline_ctx *ctx = (struct pipeline_ctx*)user_data;

#if USE_TX_IRQ
	sai_disable_irq(&ctx->dev[0], false, true);
#else
	sai_disable_irq(&ctx->dev[0], true, false);
#endif

	ctx->stats.callback++;

	ctx->event_send(ctx->event_data, status);
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

		if (ctx->id == 0) {
#if USE_TX_IRQ
			sai_enable_irq(&ctx->dev[0], false, true);
#else
			sai_enable_irq(&ctx->dev[0], true, false);
#endif
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

static void pll_adjust_disable(struct pipeline_ctx *ctx)
{
	struct hrpn_cmd_audio_element_pll cmd;

	/* need to disable PLL audio element */
	cmd.u.common.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_DISABLE;
	cmd.u.common.pipeline.id = 0;
	cmd.u.common.element.type = AUDIO_ELEMENT_PLL;
	cmd.u.common.element.id = 0;

	audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&cmd, sizeof(cmd), NULL);
}

static void pll_adjust_set_pll_id(struct pipeline_ctx *ctx, uint32_t pll_id)
{
	struct hrpn_cmd_audio_element_pll cmd;

	/* PLL element needs to know the sampling rate to determine the input PLL */
	cmd.u.common.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_PLL_ID;
	cmd.u.common.pipeline.id = 0;
	cmd.u.common.element.type = AUDIO_ELEMENT_PLL;
	cmd.u.common.element.id = 0;
	cmd.pll_id = pll_id;

	audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&cmd, sizeof(cmd), NULL);
}

static void sai_setup(struct pipeline_ctx *ctx)
{
	struct sai_cfg sai_config;
	bool pll_disable = true;
	int i;

	log_info("enter\n");

	/* Configure each active SAI */
	for (i = 0; i < sai_active_list_nelems; i++) {
		uint32_t sai_clock_root, pll_id;
		int sai_id;
		enum codec_id cid;
		int32_t ret;

		sai_config.sai_base = sai_active_list[i].sai_base;
		sai_config.bit_width = ctx->bit_width;
		sai_config.sample_rate = ctx->sample_rate;
		sai_config.chan_numbers = ctx->chan_numbers;

		sai_id = get_sai_id(sai_active_list[i].sai_base);
		os_assert(sai_id, "SAI%d enabled but not supported in this platform!", i);

		sai_clock_root = get_sai_clock_root(sai_id - 1);
		sai_config.source_clock_hz = CLOCK_GetPllFreq(sai_active_list[i].audio_pll) / CLOCK_GetRootPreDivider(sai_clock_root) / CLOCK_GetRootPostDivider(sai_clock_root);

		sai_config.tx_sync_mode = sai_active_list[i].tx_sync_mode;
		sai_config.rx_sync_mode = sai_active_list[i].rx_sync_mode;
		sai_config.msel = sai_active_list[i].msel;

		if (i == 0) {
			/* First SAI instance used as IRQ source */
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
				/* First SAI in the list manages interrupts: it cannot be
				 * slave if no codec is connected
				 */
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

static void sai_close(struct pipeline_ctx *ctx)
{
	int i;

	/* Close each active SAI */
	for (i = 0; i < sai_active_list_nelems; i++) {
		sai_drv_exit(&ctx->dev[i]);
	}
}

#if (CONFIG_GENAVB_ENABLE == 1)

static void listener_disconnect(unsigned int stream_index)
{
	struct hrpn_cmd_audio_element_avtp_disconnect disconnect;

	/* need to disconnect streams in AVTP audio element */
	disconnect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_DISCONNECT;
	disconnect.pipeline.id = 0;
	disconnect.element.type = AUDIO_ELEMENT_AVTP_SOURCE;
	disconnect.element.id = 0;
	disconnect.stream_index = stream_index;

	audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&disconnect, sizeof(disconnect), NULL);
}

static void listener_connect(struct genavb_msg_media_stack_connect *media_stack_connect)
{
	struct hrpn_cmd_audio_element_avtp_connect connect;

	/* need to connect streams in AVTP audio element */
	connect.type = HRPN_CMD_TYPE_AUDIO_ELEMENT_AVTP_SOURCE_CONNECT;
	connect.pipeline.id = 0;
	connect.element.type = AUDIO_ELEMENT_AVTP_SOURCE;
	connect.element.id = 0;
	connect.stream_index = media_stack_connect->stream_index;
	connect.stream_params = media_stack_connect->stream_params;

	audio_pipeline_ctrl((struct hrpn_cmd_audio_pipeline *)&connect, sizeof(connect), NULL);
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
			log_warn("talker not supported\n");

		break;

	case GENAVB_MSG_MEDIA_STACK_DISCONNECT:

		media_stack_disconnect = &msg.media_stack_disconnect;

		log_info("GENAVB_MSG_MEDIA_STACK_DISCONNECT stream index: %u\n",
			media_stack_disconnect->stream_index);

		if (media_stack_disconnect->direction == AVTP_DIRECTION_LISTENER)
			listener_disconnect(media_stack_disconnect->stream_index);
		else
			log_warn("talker not supported\n");

		break;

	default:
		log_err("Error, unknown message type: %d\n", msg_type);
		rc = -1;
		break;
	}

exit:
	return;
}

void play_pipeline_ctrl(void *handle)
{
	struct pipeline_ctx *ctx = handle;

	handle_avdecc_event(ctx, ctx->avb.ctrl_h);
}

static void avb_setup(struct pipeline_ctx *ctx)
{
	int genavb_result;

	log_info("enter\n");

	avb_hardware_init();

	os_irq_register(BOARD_NET_PORT0_DRV_IRQ0, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ0_HND, NULL, 0);

#ifdef BOARD_NET_PORT0_DRV_IRQ1_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ1, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ1_HND, NULL, 0);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ2_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ2, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ2_HND, NULL, 0);
#endif
#ifdef BOARD_NET_PORT0_DRV_IRQ3_HND
	os_irq_register(BOARD_NET_PORT0_DRV_IRQ3, (void (*)(void(*)))BOARD_NET_PORT0_DRV_IRQ3_HND, NULL, 0);
#endif

	os_irq_register(BOARD_GPT_0_IRQ, (void (*)(void(*)))BOARD_GPT_0_IRQ_HANDLER, NULL, 0);
	os_irq_register(BOARD_GPT_1_IRQ, (void (*)(void(*)))BOARD_GPT_1_IRQ_HANDLER, NULL, 0);

	if (gavb_stack_init()) {
		log_err("gavb_stack_init() failed\n");
		goto exit;
	}

	/* open avdecc control channel */
	genavb_result = genavb_control_open(get_genavb_handle(), &ctx->avb.ctrl_h, GENAVB_CTRL_AVDECC_MEDIA_STACK);
	if (genavb_result != GENAVB_SUCCESS) {
		log_err("genavb_control_open() failed: %s\n", genavb_strerror(genavb_result));
		goto exit;
	}

	if (STATS_TaskInit(NULL, NULL, STATS_PERIOD_MS) < 0)
		log_err("STATS_TaskInit() failed\n");

exit:
	return;
}

static void avb_close(struct pipeline_ctx *ctx)
{
	genavb_control_close(ctx->avb.ctrl_h);

	gavb_port_stats_exit(0);

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

	os_irq_unregister(BOARD_GPT_0_IRQ);
	os_irq_unregister(BOARD_GPT_1_IRQ);

	avb_hardware_exit();
}
#endif /* #if (CONFIG_GENAVB_ENABLE == 1) */

void *play_pipeline_init(void *parameters)
{
	struct audio_config *cfg = parameters;
	struct audio_pipeline_config *pipeline_cfg;
	struct pipeline_ctx *ctx;
	size_t period = DEFAULT_PERIOD;
	uint32_t rate = DEFAULT_SAMPLE_RATE;

	if (assign_nonzero_valid_val(period, cfg->period, supported_period) != 0) {
		log_err("Period %d frames is not supported\n", cfg->period);
		goto err;
	}

	if (assign_nonzero_valid_val(rate, cfg->rate, supported_rate) != 0) {
		log_err("Rate %d Hz is not supported\n", cfg->rate);
		goto err;
	}

	ctx = os_malloc(sizeof(struct pipeline_ctx) + sizeof(struct audio_pipeline_config));
	os_assert(ctx, "Audio pipeline failed with memory allocation error");
	memset(ctx, 0, sizeof(struct pipeline_ctx));

	pipeline_cfg = (struct audio_pipeline_config *)(ctx + 1);

	memcpy(pipeline_cfg, cfg->data, sizeof(struct audio_pipeline_config));

	/* override pipeline configuration */
	pipeline_cfg->sample_rate = rate;
	pipeline_cfg->period = period;
	pipeline_cfg->id = cfg->pipeline_id;

	ctx->id = cfg->pipeline_id;
	ctx->pipeline = audio_pipeline_init(pipeline_cfg);
	if (!ctx->pipeline)
		goto err_init;

	ctx->sample_rate = rate;
	ctx->chan_numbers = DEMO_AUDIO_DATA_CHANNEL;
	ctx->bit_width = DEMO_AUDIO_BIT_WIDTH;
	ctx->period = period;
	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;
	ctx->async_sem = cfg->async_sem;

	/* Only the first pipeline need to setup SAI hardware */
	if (!cfg->pipeline_id)
		sai_setup(ctx);

#if (CONFIG_GENAVB_ENABLE == 1)
	avb_setup(ctx);
#endif

	log_info("Starting %s (Sample Rate: %d Hz, Period: %u frames)\n",
			pipeline_cfg->name, rate, (uint32_t)period);

	return ctx;

err_init:
	os_free(ctx);

err:
	return NULL;
}

void play_pipeline_exit(void *handle)
{
	struct pipeline_ctx *ctx = handle;
	int i;

	audio_pipeline_exit(ctx->pipeline);

#if (CONFIG_GENAVB_ENABLE == 1)
	avb_close(ctx);
#endif

	/* Only the first pipeline need to close hardware */
	if (!ctx->id) {
		sai_close(ctx);

		for (i = 0; i < sai_active_list_nelems; i++)
			codec_close(sai_active_list[i].cid);
	}

	os_free(ctx);

	log_info("\nEnd.\n");
}
