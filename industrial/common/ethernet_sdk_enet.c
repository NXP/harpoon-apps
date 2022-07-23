/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "ethernet.h"
#include "hlog.h"
#include "hrpn_ctrl.h"
#include "industrial.h"

#include "board.h"

#include "os/irq.h"
#include "os/stdlib.h"
#include "os/string.h"
#include "os/unistd.h"

#define ENET_PHY_ADDRESS                BOARD_PHY0_ADDRESS
#define ENET_PHY_MIIMODE                BOARD_NET_PORT0_MII_MODE
/* MDIO operations. */
#define ENET_MDIO_OPS                   BOARD_PHY0_MDIO_OPS
/* PHY operations. */
#define ENET_PHY_OPS                    BOARD_PHY0_OPS

#define NONCACHEABLE(var, alignbytes)   AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes)

#define ENET_RXBD_NUM               (4)
#define ENET_TXBD_NUM               (4)
#define ENET_RXBUFF_SIZE            (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE            (ENET_FRAME_MAX_FRAMELEN)
#define ENET_DATA_LENGTH            (1000)
#define ENET_TRANSMIT_DATA_NUM      (20)
#ifndef APP_ENET_BUFF_ALIGNMENT
#define APP_ENET_BUFF_ALIGNMENT     ENET_BUFF_ALIGNMENT
#endif
#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT  (10000)
#endif

#define TEST_LOOP_COUNT             (100)

/* Buffer descriptors should be in non-cacheable region and should be align to "ENET_BUFF_ALIGNMENT". */
NONCACHEABLE(static enet_rx_bd_struct_t enet_rx_buffer_desc[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);
NONCACHEABLE(static enet_tx_bd_struct_t enet_tx_buffer_desc[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);

/*
 * The data buffers can be in cacheable region or in non-cacheable region.
 * If use cacheable region, the alignment size should be the maximum size of "CACHE LINE SIZE" and "ENET_BUFF_ALIGNMENT"
 * If use non-cache region, the alignment size is the "ENET_BUFF_ALIGNMENT".
 */
SDK_ALIGN(static uint8_t enet_rx_buffer[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)], ENET_BUFF_ALIGNMENT);
SDK_ALIGN(static uint8_t enet_tx_buffer[ENET_TXBD_NUM][SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)], ENET_BUFF_ALIGNMENT);

static enet_handle_t enet_handle;
static uint8_t tx_frame[ENET_DATA_LENGTH + 14];
static uint8_t rx_frame[ENET_DATA_LENGTH + 14];

/* Enet PHY and MDIO interface handler */
static mdio_handle_t mdio_handle = {.ops = ENET_MDIO_OPS};
static phy_handle_t phy_handle = {.phyAddr = ENET_PHY_ADDRESS, .mdioHandle = &mdio_handle, .ops = ENET_PHY_OPS};

/* Build Frame for transmit. */
static void enet_build_broadcast_frame(struct ethernet_ctx *ctx)
{
    uint32_t i;
    uint32_t length = ENET_DATA_LENGTH - 14;

    memset(&tx_frame[0], 0xff, 6);
    memcpy(&tx_frame[6], ctx->mac_addr, 6U);
    tx_frame[12] = (length >> 8) & 0xFFU;
    tx_frame[13] = length & 0xFFU;

    for (i = 0; i < length; i++)
        tx_frame[i + 14] = i % 0xFFU;
}

static int32_t enet_send(void)
{
    bool link = false;
    status_t status;

    status = PHY_GetLinkStatus(&phy_handle, &link);

    if ((status != kStatus_Success ) || !link)
        return -1;

    status = ENET_SendFrame(ENET, &enet_handle, &tx_frame[0], ENET_DATA_LENGTH, 0, false, NULL);

    return (status == kStatus_Success) ? 0 : -1;
}

static int32_t enet_receive(uint32_t *length)
{
    status_t status;
    enet_data_error_stats_t err_static;

    /* Get the Frame size */
    status = ENET_GetRxFrameSize(&enet_handle, length, 0);
    if (*length) {
        status = ENET_ReadFrame(ENET, &enet_handle, &rx_frame[0], *length, 0, NULL);
        if (status == kStatus_Success)
            return 0;
        else
            return -1;
    } else if (status == kStatus_ENET_RxFrameError) {
        /* Update the received buffer when error happened. */
        /* Get the error information of the received frame. */
        ENET_GetRxErrBeforeReadFrame(&enet_handle, &err_static, 0);
        ENET_ReadFrame(ENET, &enet_handle, NULL, 0, 0, NULL);
        return -1;
    }

    return 0;
}

void ethernet_sdk_enet_stats(void *priv)
{
	struct ethernet_ctx *ctx = priv;

	(void)ctx;
	log_info("not implemented\n");
}

int ethernet_sdk_enet_run(void *priv, struct event *e)
{
    struct ethernet_ctx *ctx = priv;

    log_info("\n");

    uint32_t tx_pass_cnt, rx_pass_cnt, rx_err_cnt, rx_empty_cnt;
    enet_buffer_config_t buffer_config = {0};
    uint32_t enet_ipg_freq, length, count;
    phy_config_t phy_config = {0};
    bool autonego = false;
    enet_config_t config;
    phy_duplex_t duplex;
    phy_speed_t speed;
    bool link = false;
    status_t status;
    int32_t ret;

    os_printf("\r\nENET test start.\r\n");

    /* Prepare the buffer configuration. */
    buffer_config.rxBdNumber = ENET_RXBD_NUM;
    buffer_config.txBdNumber = ENET_TXBD_NUM;
    buffer_config.rxBuffSizeAlign = SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT);
    buffer_config.txBuffSizeAlign = SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT);
    buffer_config.rxBdStartAddrAlign = &enet_rx_buffer_desc[0];
    buffer_config.txBdStartAddrAlign = &enet_tx_buffer_desc[0];
    buffer_config.rxBufferAlign = &enet_rx_buffer[0][0];
    buffer_config.txBufferAlign = &enet_tx_buffer[0][0];
    buffer_config.rxMaintainEnable = true;
    buffer_config.txMaintainEnable = true;
    buffer_config.txFrameInfo = NULL;

    enet_ipg_freq = CLOCK_GetFreq(kCLOCK_EnetIpgClk);

    ENET_GetDefaultConfig(&config);

    /* Enable promiscuous mode */
    config.macSpecialConfig |= kENET_ControlPromiscuousEnable;
    /* The miiMode should be set according to the different PHY interfaces. */
    config.miiMode = ENET_PHY_MIIMODE;

    phy_config.phyAddr = ENET_PHY_ADDRESS;
    phy_config.autoNeg = true;
    mdio_handle.resource.base = ENET;
    mdio_handle.resource.csrClock_Hz = enet_ipg_freq;

    /* Initialize PHY */
    status = PHY_Init(&phy_handle, &phy_config);
    if (status != kStatus_Success) {
        os_printf("ENET: PHY init failed!\r\n");
        goto enet_fail;
    }

    os_printf("ENET: Wait for PHY link up...\r\n");
    /* Wait for auto-negotiation success and link up */
    count = PHY_AUTONEGO_TIMEOUT_COUNT;
    do {
        PHY_GetAutoNegotiationStatus(&phy_handle, &autonego);
        PHY_GetLinkStatus(&phy_handle, &link);
        if (autonego && link)
            break;
        os_msleep(1);
    } while (--count);

    if (!count) {
        os_printf("ENET: PHY Auto-negotiation (%s) and link (%s)\r\n",
                  autonego ? "compeleted" : "failed", link ? "up" : "down");
        goto enet_fail;
    }

    /* Get the actual PHY link speed. */
    PHY_GetLinkSpeedDuplex(&phy_handle, &speed, &duplex);
    os_printf("ENET: PHY link speed %s %s-duplex\r\n",
              speed ? ((speed == 2) ? "1000M" : "100M") : "10MB", duplex ? "full" : "half");

    /* Change the MII speed and duplex for actual link status. */
    config.miiSpeed  = (enet_mii_speed_t)speed;
    config.miiDuplex = (enet_mii_duplex_t)duplex;
    ENET_Init(ENET, &enet_handle, &config, &buffer_config, ctx->mac_addr,  enet_ipg_freq);
    ENET_ActiveRead(ENET);

    enet_build_broadcast_frame(ctx);

    tx_pass_cnt = rx_pass_cnt = rx_err_cnt = rx_empty_cnt = 0;
    for (count = 0; count < TEST_LOOP_COUNT; count++) {
        /* Transmit frame */
        ret = enet_send();
        if (!ret)
            tx_pass_cnt++;

        os_msleep(50);

        /* Receive frame */
        ret = enet_receive(&length);
	if (!ret) {
            if (!length)
                rx_empty_cnt++;
            else
                rx_pass_cnt++;
	} else {
            rx_err_cnt++;
	}
    }

    os_printf("\r\nENET test result:\r\n");
    os_printf("\tTX: total = %d; succ = %d; fail = %d\r\n",
              TEST_LOOP_COUNT, tx_pass_cnt, TEST_LOOP_COUNT - tx_pass_cnt);
    os_printf("\tRX: total = %d; succ = %d; fail = %d; empty = %d\r\n",
              TEST_LOOP_COUNT, rx_pass_cnt, rx_err_cnt, rx_empty_cnt);

    return 0;

enet_fail:
    os_printf("\r\nENET test failed!\r\n");

    return -1;
}

void *ethernet_sdk_enet_init(void *parameters)
{
	struct industrial_config *cfg = parameters;
	struct ethernet_ctx *ctx;

	ctx = os_malloc(sizeof(struct ethernet_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");

		goto exit;
	}

	memset(ctx, 0, sizeof(struct ethernet_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	memcpy(ctx->mac_addr, cfg->address, sizeof(ctx->mac_addr));

	log_info("%s\n", __func__);

exit:
	return ctx;
}

void ethernet_sdk_enet_exit(void *priv)
{
	struct ethernet_ctx *ctx = priv;

	os_free(ctx);

	log_info("end\n");
}
