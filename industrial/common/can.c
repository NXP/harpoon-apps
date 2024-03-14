/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "hardware_flexcan.h"
#include "hlog.h"
#include "industrial.h"
#include "fsl_flexcan.h"

#include "os/counter.h"
#include "os/irq.h"
#include "os/stdlib.h"
#include "os/string.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USE_IMPROVED_TIMING_CONFIG (1U)

/*
 *    DWORD_IN_MB    DLC    BYTES_IN_MB             Maximum MBs
 *    2              8      kFLEXCAN_8BperMB        64
 *    4              10     kFLEXCAN_16BperMB       42
 *    8              13     kFLEXCAN_32BperMB       25
 *    16             15     kFLEXCAN_64BperMB       14
 *
 * Dword in each message buffer, Length of data in bytes, Payload size must align,
 * and the Message Buffers are limited corresponding to each payload configuration:
 */
#define DLC			(8)
#define BYTES_IN_MB	kFLEXCAN_8BperMB
#define DWORD_IN_MB	(2)
#define RX_MB_ID_MASK	0xFFUL

#define MAX_NODES					2
#define MAX_MESSAGE_BUFFERS			4
#define PROCESS_ALARM_PERIOD_US		1200U
#define MEGA						1000000U

extern os_counter_t *os_counter;

enum {
	TEST_4_MB = 0,
};

struct mb_config {
	bool tx;
	uint32_t index;
	uint32_t frame_id;
};

struct message_buffer {
	union {
		flexcan_fd_frame_t canfd_frame;
		flexcan_frame_t frame;
	} u;

	uint8_t data_bytes[16];
	struct mb_config conf;
	bool on_hold;

	union {
		struct _tx_stats {
			uint32_t w_success;
			uint32_t w_fail;
			uint32_t irq_iter;
			uint32_t busy;
		} tx;

		struct _rx_stats {
			uint32_t r_success;
			uint32_t r_fail;
			uint32_t irq_iter;
			uint32_t overflow;
		} rx;
	} stats;
};

struct node_config {
	uint8_t message_buffer_n;
	struct mb_config mb_conf[MAX_MESSAGE_BUFFERS];
};

struct can_ctx {
	void (*event_send)(void *, uint8_t, uint8_t);
	void (*canHandler)(void *);
	void *event_data;

	CAN_Type *base;
	uint32_t clk_freq;
	uint32_t bps;
	uint32_t global_irq_mask;
	uint64_t global_irq_count;

	/* Activated the given number message buffers */
	struct message_buffer mb[MAX_MESSAGE_BUFFERS];
	uint8_t mb_number;

	uint8_t node;
	uint8_t test_type;

	struct os_counter_alarm_cfg alarm_cfg;
	uint32_t alarm_err;

	bool use_canfd;
};

static const struct node_config node_configs[MAX_NODES] = {
	[0] = { /* Node 0 config*/
		.message_buffer_n = 4,
		.mb_conf = {
			[0] = {
				.tx = true,
				.index = 1,
				.frame_id = 0x123,
			},
			[1] = {
				.tx = false,
				.index = 2,
				.frame_id = 0x321,
			},
			[2] = {
				.tx = true,
				.index = 3,
				.frame_id = 0x124,
			},
			[3] = {
				.tx = false,
				.index = 4,
				.frame_id = 0x422,
			}
		}
	},
	[1] = { /*Node 1 config*/
		.message_buffer_n = 4,
		.mb_conf = {
			[0] = {
				.tx = true,
				.index = 1,
				.frame_id = 0x321,
			},
			[1] = {
				.tx = false,
				.index = 2,
				.frame_id = 0x123,
			},
			[2] = {
				.tx = true,
				.index = 3,
				.frame_id = 0x422,
			},
			[3] = {
				.tx = false,
				.index = 4,
				.frame_id = 0x124,
			}
		}
	}
};

static void can_irq_handler(void *data)
{
	struct can_ctx *ctx = data;

	FLEXCAN_DisableMbInterrupts(ctx->base, ctx->global_irq_mask);

	if (ctx->event_send)
		ctx->event_send(ctx->event_data, EVENT_TYPE_IRQ, 0);
}

static void alarm_handler(os_counter_t *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	struct can_ctx *ctx = user_data;

	if (ctx->event_send)
		ctx->event_send(ctx->event_data, EVENT_TYPE_TIMER, 0);

}

static int alarm_trigger(struct can_ctx *ctx, uint32_t us)
{
	int err = 0;
	uint64_t ticks = os_counter_us_to_ticks(os_counter, us);

	ctx->alarm_cfg.ticks = ticks;
	ctx->alarm_cfg.flags = 0;
	ctx->alarm_cfg.user_data = ctx;
	ctx->alarm_cfg.callback = alarm_handler;

	err = os_counter_set_channel_alarm(os_counter, 0, &ctx->alarm_cfg);

	/* Counter set alarm failed */
	if (err < 0) {
		ctx->alarm_err++;
	}

	return err;
}

uint32_t get_global_irq_mask(struct node_config node, uint8_t message_buffer_number)
{
	uint32_t mask = 0;

	for (int i = 0; i < message_buffer_number; i++)
		mask |= (uint32_t)1U << node.mb_conf[i].index;

	return mask;
}

static void can_enable_interrupt(struct can_ctx *ctx)
{
	os_irq_register(EXAMPLE_FLEXCAN_IRQn, ctx->canHandler, ctx, OS_IRQ_PRIO_DEFAULT + 1);
	os_irq_enable(EXAMPLE_FLEXCAN_IRQn);
	FLEXCAN_EnableMbInterrupts(ctx->base, ctx->global_irq_mask);
}

static void can_disable_interrupt(struct can_ctx *ctx)
{
	FLEXCAN_DisableMbInterrupts(ctx->base, ctx->global_irq_mask);
	os_irq_disable(EXAMPLE_FLEXCAN_IRQn);
	os_irq_unregister(EXAMPLE_FLEXCAN_IRQn);
}

static int can_config(struct can_ctx *ctx)
{
	flexcan_config_t flexcanConfig;
	int status = 0;

	FLEXCAN_GetDefaultConfig(&flexcanConfig);
	flexcanConfig.enableIndividMask = true;

	if (ctx->use_canfd)
		ctx->bps = flexcanConfig.baudRateFD;
	else
		ctx->bps = flexcanConfig.baudRate;

/* Use the FLEXCAN API to automatically get the ideal bit timing configuration. */
#if (defined(USE_IMPROVED_TIMING_CONFIG) && USE_IMPROVED_TIMING_CONFIG)

	flexcan_timing_config_t timing_config;
	memset(&timing_config, 0, sizeof(flexcan_timing_config_t));
	if (ctx->use_canfd && FLEXCAN_FDCalculateImprovedTimingValues(ctx->base, flexcanConfig.baudRate, flexcanConfig.baudRateFD,
																  ctx->clk_freq, &timing_config)) {
		/* Update the improved timing configuration*/
		memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
	} else if (!ctx->use_canfd && FLEXCAN_CalculateImprovedTimingValues(ctx->base, flexcanConfig.baudRate, ctx->clk_freq,
																	  &timing_config)) {
		/* Update the improved timing configuration*/
		memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
	} else {
		log_info("Improved Timing Configuration not found. Used default configuration\n\n");
	}
#endif

	log_info("Init FlexCAN with clock freq %lu Hz\n", ctx->clk_freq );
	FLEXCAN_Init(ctx->base, &flexcanConfig, ctx->clk_freq);
	if (ctx->use_canfd) {
		if (FLEXCAN_IsInstanceHasFDMode(ctx->base)) {
			FLEXCAN_FDInit(ctx->base, &flexcanConfig, ctx->clk_freq , BYTES_IN_MB, true);
		} else {
			log_err("CAN FD not supported\n");
			status  = -1;
		}
	}

	return status;
}

static void mb_tx_setup(struct message_buffer *mb, CAN_Type *base, bool is_canfd)
{
	if (is_canfd)
		FLEXCAN_SetFDTxMbConfig(base, mb->conf.index, true);
	else
		FLEXCAN_SetTxMbConfig(base, mb->conf.index, true);
}

static void mb_rx_setup(struct message_buffer *mb, CAN_Type *base, bool is_canfd)
{
	flexcan_rx_mb_config_t mbConfig;

	/* Setup Rx Message Buffer. */
	mbConfig.format = kFLEXCAN_FrameFormatStandard;
	mbConfig.type   = kFLEXCAN_FrameTypeData;
	mbConfig.id     = FLEXCAN_ID_STD(RX_MB_ID_MASK & mb->conf.frame_id);

	/* Setup Rx individual ID mask. */
	FLEXCAN_SetRxIndividualMask(base, mb->conf.index,
								FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK, 0, 0));

	/* Setup Rx Message Buffer. */
	if (is_canfd)
		FLEXCAN_SetFDRxMbConfig(base, mb->conf.index, &mbConfig, true);
	else
		FLEXCAN_SetRxMbConfig(base, mb->conf.index, &mbConfig, true);
}

static int mb_write(struct message_buffer *mb, CAN_Type *base, bool is_canfd)
{
	if (is_canfd) {
		mb->u.canfd_frame.id     = FLEXCAN_ID_STD(mb->conf.frame_id);
		mb->u.canfd_frame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
		mb->u.canfd_frame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
		mb->u.canfd_frame.length = (uint8_t)DLC;
		mb->u.canfd_frame.brs = (uint8_t)1U;
		mb->u.canfd_frame.dataByte0 = 0x1;
		mb->u.canfd_frame.dataByte1 = 0x75;

		return FLEXCAN_WriteFDTxMb(base, mb->conf.index, &mb->u.canfd_frame);
	}

	mb->u.frame.id     = FLEXCAN_ID_STD(mb->conf.frame_id);
	mb->u.frame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
	mb->u.frame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
	mb->u.frame.length = (uint8_t)DLC;
	mb->u.frame.dataByte0 = 0x2;
	mb->u.frame.dataByte1 = 0x55;

	return FLEXCAN_WriteTxMb(base, mb->conf.index, &mb->u.frame);
}

static int mb_read(struct message_buffer *mb, CAN_Type *base, bool is_canfd)
{
	if (is_canfd)
		return FLEXCAN_ReadFDRxMb(base, mb->conf.index, &mb->u.canfd_frame);

	return FLEXCAN_ReadRxMb(base, mb->conf.index, &mb->u.frame);
}

static void mb_tx_process(struct message_buffer *mb, CAN_Type *base, bool is_canfd, struct event *e)
{
	int status;

	switch (e->type) {
	case EVENT_TYPE_START:
		mb_tx_setup(mb, base, is_canfd);
		break;
	case EVENT_TYPE_IRQ:
		mb->on_hold = false;
		mb->stats.tx.irq_iter++;
		break;
	case EVENT_TYPE_TIMER:
		if (!mb->on_hold) {
			status = mb_write(mb, base, is_canfd);
			if (status == kStatus_Success) {
				mb->on_hold = true;
				mb->stats.tx.w_success++;
			}
			else {
				mb->stats.tx.w_fail++;
			}
		} else {
			mb->stats.tx.busy++;
		}
		break;
	default:
		break;
	}
}

static void mb_rx_process(struct message_buffer *mb, CAN_Type *base, bool is_canfd, struct event *e)
{
	int status;

	switch (e->type) {
	case EVENT_TYPE_START:
		mb_rx_setup(mb, base, is_canfd);
		break;
	case EVENT_TYPE_IRQ:
		mb->stats.rx.irq_iter++;
		status = mb_read(mb, base, is_canfd);
		if (status == kStatus_Success)
			mb->stats.rx.r_success++;
		else if (status == kStatus_FLEXCAN_RxOverflow)
			mb->stats.rx.overflow++;
		else
			mb->stats.rx.r_fail++;
		break;
	default:
		break;
	}
}

static void *get_ctx(void *parameters, uint32_t test_type)
{
	struct industrial_config *cfg = parameters;
	uint32_t node = cfg->role;
	struct can_ctx *ctx = NULL;

	/* Sanity check */
	if (node >= MAX_NODES) {
		log_err("Invalid role (node type): %u\n", node);
		goto exit;
	}

	ctx = os_malloc(sizeof(struct can_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");
		goto exit;
	}

	memset(ctx, 0, sizeof(struct can_ctx));

	ctx->base = EXAMPLE_CAN;
	ctx->clk_freq = EXAMPLE_CAN_CLK_FREQ;

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;
	ctx->use_canfd = cfg->protocol;

	/* User input values */
	ctx->node = node;
	ctx->test_type = test_type;
	ctx->mb_number = MAX_MESSAGE_BUFFERS;

	if (ctx->mb_number > node_configs[ctx->node].message_buffer_n) {
		log_err("The given message buffer number is unsupported\n");
		goto exit;
	}

	/* Process values */
	ctx->canHandler = can_irq_handler;
	ctx->global_irq_mask = get_global_irq_mask(node_configs[ctx->node], ctx->mb_number);
	ctx->global_irq_count = 0;

	/* Load message buffers config */
	for (int i = 0; i < ctx->mb_number; i++)
		ctx->mb[i].conf = node_configs[ctx->node].mb_conf[i];

exit:
	return ctx;
}

void *can_init(void *parameters)
{
	struct can_ctx *ctx;

	log_info("START\n");
	ctx = get_ctx(parameters, TEST_4_MB);
	if (!ctx) {
		log_err("Context not found\n");
		return NULL;
	}

	hardware_flexcan_init();
	if (can_config(ctx) < 0)
		goto err_config;

	return ctx;

err_config:
	FLEXCAN_Deinit(ctx->base);
	os_free(ctx);

	return NULL;
}

int can_run(void *priv, struct event *e)
{
	struct can_ctx *ctx = priv;
	uint8_t status = -1;
	uint64_t flag = 0;

	if (!ctx) {
		log_err("Context not found\n");
		return status;
	}

	switch (e->type) {
	case EVENT_TYPE_START:
		for (int i= 0; i < ctx->mb_number; i++) {
			if (node_configs[ctx->node].mb_conf[i].tx) {
				mb_tx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
			} else {
				mb_rx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
			}
		}
		can_enable_interrupt(ctx);
		os_counter_start(os_counter);

		if (alarm_trigger(ctx, PROCESS_ALARM_PERIOD_US) < 0)
			break;

		status = 0;
		break;
	case EVENT_TYPE_IRQ:
		ctx->global_irq_count++;
		flag = FLEXCAN_GetMbStatusFlags(ctx->base, ctx->global_irq_mask);
		FLEXCAN_ClearMbStatusFlags(ctx->base, flag);

		for (int i = 0; i < ctx->mb_number; i++) {
			/* Mailbox (index = N) is defined in mb[N-1] */
			if (flag & (1ULL << (i + 1))) {
				if (ctx->mb[i].conf.tx)
					mb_tx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
				else
					mb_rx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
			}
		}
		FLEXCAN_EnableMbInterrupts(ctx->base, ctx->global_irq_mask);

		status = 0;
		break;
	case EVENT_TYPE_TIMER:
		for (int i= 0; i < ctx->mb_number; i++) {
			if (ctx->mb[i].conf.tx)
				mb_tx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
		}

		if (alarm_trigger(ctx, PROCESS_ALARM_PERIOD_US) < 0)
			break;

		status = 0;
		break;
	default:
		log_err("Invalid event: %u\n", e->type);
		break;
	}

	return status;
}

void can_stats(void *priv)
{
	struct can_ctx *ctx = priv;

	log_info("|Mbit/s: %9u|TX period us: %5u|global irq: %+u|\n", ctx->bps/MEGA, PROCESS_ALARM_PERIOD_US, ctx->global_irq_count);
	for (int i= 0; i < ctx->mb_number; i++) {
		if (ctx->mb[i].conf.tx) {
			log_info("|TX mb: %u, id: %x|==>|irq: %10u|tx: %10u|busy  : %5u|fail: %3u|\n",
					ctx->mb[i].conf.index, ctx->mb[i].conf.frame_id, ctx->mb[i].stats.tx.irq_iter,
					ctx->mb[i].stats.tx.w_success, ctx->mb[i].stats.tx.busy, ctx->mb[i].stats.tx.w_fail);
		} else {
			log_info("|RX mb: %u, id: %x|==>|irq: %10u|rx: %10u|ovrflw: %5u|fail: %3u|\n",
					ctx->mb[i].conf.index, ctx->mb[i].conf.frame_id, ctx->mb[i].stats.rx.irq_iter,
					ctx->mb[i].stats.rx.r_success, ctx->mb[i].stats.rx.overflow, ctx->mb[i].stats.rx.r_fail);
		}
	}
}

void can_exit(void *priv)
{
	struct can_ctx *ctx = priv;

	os_counter_stop(os_counter);
	os_counter_cancel_channel_alarm(os_counter, 0);
	can_disable_interrupt(ctx);
	FLEXCAN_Deinit(ctx->base);
	os_free(ctx);

	log_info("END\n");
}
