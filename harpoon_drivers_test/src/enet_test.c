/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_enet.h"
#include "os/stdio.h"
#include "os/unistd.h"

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

#ifndef MAC_ADDRESS
#define MAC_ADDRESS                 {0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x60}
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

/* The MAC address for ENET device. */
static uint8_t enet_mac_addr[6] = MAC_ADDRESS;

/* Enet PHY interface handler */
static phy_handle_t phy_handle;

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(ENET, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(ENET, phyAddr, regAddr, pData);
}

static void MDIO_Init(uint32_t enet_ipg_freq)
{
    ENET_SetSMI(ENET, enet_ipg_freq, false);

    phy_resource.write = MDIO_Write;
    phy_resource.read = MDIO_Read;
}

/* Build Frame for transmit. */
static void enet_build_broadcast_frame(void)
{
    uint32_t i;
    uint32_t length = ENET_DATA_LENGTH - 14;

    memset(&tx_frame[0], 0xff, 6);
    memcpy(&tx_frame[6], &enet_mac_addr[0], 6U);
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

void enet_test(void)
{
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

    enet_ipg_freq = CLOCK_GetFreq(kCLOCK_EnetIpgClk);

    MDIO_Init(enet_ipg_freq);

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


    ENET_GetDefaultConfig(&config);

    /* Enable promiscuous mode */
    config.macSpecialConfig |= kENET_ControlPromiscuousEnable;
    /* The miiMode should be set according to the different PHY interfaces. */
    config.miiMode = ENET_PHY_MIIMODE;

    phy_config.phyAddr = ENET_PHY_ADDRESS;
    phy_config.autoNeg = true;
    phy_config.ops = &ENET_PHY_OPS;
    phy_config.resource = &phy_resource;

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
    ENET_Init(ENET, &enet_handle, &config, &buffer_config, &enet_mac_addr[0],  enet_ipg_freq);
    ENET_ActiveRead(ENET);

    enet_build_broadcast_frame();

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
    return;

enet_fail:
    os_printf("\r\nENET test failed!\r\n");
}
