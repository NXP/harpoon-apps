/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ethernet.h"
#include "app_board.h"
#include "hlog.h"
#include "hrpn_ctrl.h"
#include "industrial.h"
#include "hardware_ethernet.h"

#include "rtos_abstraction_layer.h"

#include "os/cache.h"
#include "os/irq.h"

#define ENET_PHY_MIIMODE        kENET_QOS_RgmiiMode

#define ENET_QOS_RXBD_NUM                     (4)
#define ENET_QOS_TXBD_NUM                     (4)
#define ENET_QOS_RXQUEUE_USE                  (3)
#define ENET_QOS_TXQUEUE_USE                  (3)
#define ENET_QOS_RXBUFF_SIZE                  (ENET_QOS_FRAME_MAX_FRAMELEN)
#define ENET_QOS_TXBUFF_SIZE                  (ENET_QOS_FRAME_MAX_FRAMELEN)
#define ENET_QOS_BuffSizeAlign(n)             ENET_QOS_ALIGN(n, ENET_QOS_BUFF_ALIGNMENT)
#define ENET_QOS_ALIGN(x, align)              ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))
#define ENET_QOS_DATA_LENGTH                  (1000)
#define ENET_QOS_HEAD_LENGTH                  (14)
#define ENET_QOS_FRAME_LENGTH                 (ENET_QOS_DATA_LENGTH + ENET_QOS_HEAD_LENGTH)
#define ENET_QOS_TRANSMIT_DATA_NUM            (30)
#define ENET_QOS_HEAD_TYPE_OFFSET             12U     /*!< ENET head type offset. */
#define ENET_QOS_VLANTYPE                     0x8100U /*! @brief VLAN TYPE */
#define ENET_QOS_VLANTAGLEN                   4U      /*! @brief VLAN TAG length */
#define ENET_QOS_AVBTYPE                      0x22F0U /*! @brief AVB TYPE */
#define ENET_QOS_AVTPDU_IEC61883_6AUDIOTYPE   0x10U /*! @brief AVTPDU IEC61883-6 audio&music type. */
#ifndef APP_ENET_QOS_BUFF_ALIGNMENT
#define APP_ENET_QOS_BUFF_ALIGNMENT ENET_QOS_BUFF_ALIGNMENT
#endif

#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (800000U)
#endif
#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (500000U)
#endif

/* Re-define ENET_QOS_HTONS - This needs to be aligned with ENET_QOS driver */
#define ENET_QOS_HTONS(n) __REV16(n)
#define ENET_QOS_HTONL(n) __REV(n)
#define ENET_QOS_NTOHS(n) __REV16(n)
#define ENET_QOS_NTOHL(n) __REV(n)

NONCACHEABLE enet_qos_rx_bd_struct_t g_rxBuffDescrip[ENET_QOS_RXQUEUE_USE][ENET_QOS_RXBD_NUM]  __attribute__((aligned(ENET_QOS_BUFF_ALIGNMENT)));
NONCACHEABLE enet_qos_tx_bd_struct_t g_txBuffDescrip[ENET_QOS_TXQUEUE_USE][ENET_QOS_TXBD_NUM]  __attribute__((aligned(ENET_QOS_BUFF_ALIGNMENT)));

/*! @brief The data buffers can be in cacheable region or in non-cacheable region.
 * If use cacheable region, the alignment size should be the maximum size of "CACHE LINE SIZE" and
 * "ENET_QOS_BUFF_ALIGNMENT" If use non-cache region, the alignment size is the "ENET_QOS_BUFF_ALIGNMENT".
 */

enet_qos_frame_info_t g_txDirty[ENET_QOS_TXQUEUE_USE][ENET_QOS_RXBD_NUM];
uint32_t rxbuffer[ENET_QOS_RXQUEUE_USE][ENET_QOS_RXBD_NUM];
enet_qos_handle_t g_handle = {0};
/* The MAC address for ENET device. */
uint8_t g_macAddr2[6] = {0xd3, 0xbe, 0xd9, 0x45, 0x22, 0x60};
uint8_t g_frame[ENET_QOS_TXQUEUE_USE][ENET_QOS_FRAME_LENGTH];
volatile uint32_t g_rxIndex  = 0;
volatile uint32_t g_rxIndex1 = 0;
volatile uint32_t g_rxIndex2 = 0;
volatile uint32_t g_txIndex = 0;
volatile uint32_t g_txIndex1 = 0;
volatile uint32_t g_txIndex2 = 0;
volatile uint32_t g_txSuccessFlag = false;
volatile uint32_t g_rxSuccessFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*! @brief Build Frame for transmit. */
static void ENET_QOS_BuildFrame(struct ethernet_ctx *ctx)
{
    uint8_t index  = 0;
    uint32_t count = 0;
    uint32_t offset = 0;

    /* memset */
    for (index = 0; index < ENET_QOS_TXQUEUE_USE; index++)
    {
        memset(&g_frame[index], 0, ENET_QOS_FRAME_LENGTH);
    }

    /* Create the mac addresses. */
    for (index = 0; index < 2U; index++)
    {
        for (count = 0; count < 6U; count++)
        {
            g_frame[index][count] = 0xFFU;
        }
        memcpy(&g_frame[index][6], ctx->mac_addr, 6U);
    }

    /* Create the different type frame from the ethernet type offset:
     * first: for normal frame
     * second: for Class A data flag specififc AVB frame with VLAN tag.
     */
    /* First frame - broadcast frame. */
    g_frame[0][ENET_QOS_HEAD_TYPE_OFFSET] = (ENET_QOS_DATA_LENGTH >> 8) & 0xFFU;
    g_frame[0][13]                        = ENET_QOS_DATA_LENGTH & 0xFFU;
    g_frame[0][16]                        = 0x33;
    g_frame[0][17]                        = 0x55;
    g_frame[0][18]                        = 0xDD;
    g_frame[0][19]                        = 0xEE;
    for (count = ENET_QOS_HEAD_LENGTH + 6U; count < ENET_QOS_FRAME_LENGTH; count++)
    {
        g_frame[0][count] = count % 0xFFU;
    }

    /* Second frame. */
    *(uint16_t *)&g_frame[1][ENET_QOS_HEAD_TYPE_OFFSET] = ENET_QOS_HTONS(ENET_QOS_VLANTYPE); /* VLAN TAG type. */
    *(uint16_t *)&g_frame[1][ENET_QOS_HEAD_TYPE_OFFSET + ENET_QOS_VLANTAGLEN / 2] =
        ENET_QOS_HTONS((5 << 13U)); /* Prio 5. */
    *(uint16_t *)&g_frame[1][ENET_QOS_HEAD_TYPE_OFFSET + ENET_QOS_VLANTAGLEN] =
        ENET_QOS_HTONS(ENET_QOS_AVBTYPE); /* AVTP type. */
    offset = ENET_QOS_HEAD_TYPE_OFFSET + ENET_QOS_VLANTAGLEN + 2;
    g_frame[1][offset]      = 0; /* AVTPDU set subtype with IEC61883 in common header. */
    g_frame[1][26 + offset] = 0; /* AVTPDU set SPH field. */
    g_frame[1][28 + offset] = (1 << 7) | ENET_QOS_AVTPDU_IEC61883_6AUDIOTYPE;
    for (count = 50; count < ENET_QOS_FRAME_LENGTH; count++)
    {
        g_frame[1][count] = count % 0xFFU;
    }

    /* Third frame - unicast frame. */
    memcpy(&g_frame[2][0], &g_macAddr2[0], 6U);
    memcpy(&g_frame[2][6], ctx->mac_addr, 6U);
    g_frame[2][ENET_QOS_HEAD_TYPE_OFFSET] = (ENET_QOS_DATA_LENGTH >> 8) & 0xFFU;
    g_frame[2][13] = ENET_QOS_DATA_LENGTH & 0xFFU;
    for (count = ENET_QOS_HEAD_LENGTH; count < ENET_QOS_FRAME_LENGTH; count++)
    {
        g_frame[2][count] = count % 0xFFU;
    }

    /* make sure the tx frames are written to memory before DMA bursts*/
    os_dcache_data_range((uintptr_t)g_frame[0], ENET_QOS_FRAME_LENGTH, OS_CACHE_WB);
    os_dcache_data_range((uintptr_t)g_frame[1], ENET_QOS_FRAME_LENGTH, OS_CACHE_WB);
    os_dcache_data_range((uintptr_t)g_frame[2], ENET_QOS_FRAME_LENGTH, OS_CACHE_WB);
}

static void ENET_QOS_IntCallback(
    ENET_QOS_Type *base, enet_qos_handle_t *handle, enet_qos_event_t event, uint8_t channel, void *userData)
{
    uint32_t length = 0U;
    status_t status;
    uint8_t data[ENET_QOS_RXBUFF_SIZE];

    switch (event)
    {
        case kENET_QOS_RxIntEvent:
            /* Get the Frame size */
            do
            {
                status = ENET_QOS_GetRxFrameSize(base, &g_handle, &length, channel);
                if ((status == kStatus_Success) && (length != 0))
                {
                    /* Received valid frame. Deliver the rx buffer with the size equal to length. */
                    status = ENET_QOS_ReadFrame(base, &g_handle, data, length, channel, NULL);
                    if (status == kStatus_Success)
                    {
                        if (channel == 0)
                        {
                            g_rxIndex++;
                        }
                        else if (channel == 1)
                        {
                            g_rxIndex1++;
                        }
                        else if (channel == 2)
                        {
                            g_rxIndex2++;
                        }
                    }
                }
                else if (status == kStatus_ENET_QOS_RxFrameError)
                {
                    /* update the receive buffer. */
                    ENET_QOS_ReadFrame(base, &g_handle, NULL, 0, channel, NULL);
                }
            } while (length != 0U);

            /* Set rx success flag. */
            if ((g_rxIndex + g_rxIndex1 + g_rxIndex2) == ENET_QOS_TRANSMIT_DATA_NUM)
            {
                g_rxSuccessFlag = true;
            }
            break;
        case kENET_QOS_TxIntEvent:
            switch (channel)
            {
                case 0:
                    g_txIndex++;
                    break;
                case 1:
                    g_txIndex1++;
                    break;
                case 2:
                    g_txIndex2++;
                    break;
                default:
                    break;
            }
            if ((g_txIndex + g_txIndex1 + g_txIndex2) == ENET_QOS_TRANSMIT_DATA_NUM)
            {
                g_txSuccessFlag = true;
            }
            break;
        default:
            break;
    }
}

static void enet_qos_test_irq_handler(void *data)
{
    ENET_QOS_CommonIRQHandler(EXAMPLE_ENET_QOS_BASE, &g_handle);
}

static void enet_qos_prepare_configuration(enet_qos_config_t *config, enet_qos_ptp_config_t *ptpConfig,
        enet_qos_multiqueue_config_t *multiQueue, enet_qos_rxp_config_t *rxpConfig,
        phy_handle_t *phyHandle, uint32_t refClock)
{
    phy_speed_t speed;
    phy_duplex_t duplex;

    /* Get default configuration 1000M RGMII. */
    ENET_QOS_GetDefaultConfig(config);

    PHY_GetLinkSpeedDuplex(phyHandle, &speed, &duplex);
    /* Use the actual speed and duplex when phy success to finish the autonegotiation. */
    switch (speed)
    {
        case kPHY_Speed10M:
            config->miiSpeed = kENET_QOS_MiiSpeed10M;
            break;
        case kPHY_Speed100M:
            config->miiSpeed = kENET_QOS_MiiSpeed100M;
            break;
        case kPHY_Speed1000M:
            config->miiSpeed = kENET_QOS_MiiSpeed1000M;
            break;
        default:
            break;
    }
    config->miiDuplex = (enet_qos_mii_duplex_t)duplex;
    config->miiMode = ENET_PHY_MIIMODE;
    /* Shoule enable the promiscuous mode and enable the store and forward
     * to make the timestamp is always updated correclty in the descriptors. */
    config->specialControl = kENET_QOS_PromiscuousEnable | kENET_QOS_StoreAndForward;
    config->csrClock_Hz    = EXAMPLE_ENET_QOS_CLK_FREQ;

    ptpConfig->tsRollover         = kENET_QOS_DigitalRollover;
    ptpConfig->systemTimeClock_Hz = refClock;
    config->ptpConfig             = ptpConfig;

    /* Multi-queue config */
    *multiQueue = (enet_qos_multiqueue_config_t)
    {
        .burstLen   = kENET_QOS_BurstLen1,
        .txQueueUse = ENET_QOS_TXQUEUE_USE,
        .mtltxSche  = kENET_QOS_txWeightRR,
        .txQueueConfig =
        {
            {
                .mode      = kENET_QOS_DCB_Mode,
                .weight    = 0x10U,
                .priority  = 0x0U,
                .cbsConfig = NULL,
            },
            {
                .mode      = kENET_QOS_DCB_Mode,
                .weight    = 0x10U,
                .priority  = 0x1U,
                .cbsConfig = NULL,
            },
            {
                .mode      = kENET_QOS_DCB_Mode,
                .weight    = 0x10U,
                .priority  = 0x2U,
                .cbsConfig = NULL,
            },
        },
        .rxQueueUse = ENET_QOS_RXQUEUE_USE,
        .mtlrxSche  = kENET_QOS_rxStrPrio,
        .rxQueueConfig =
        {
            {
                .mode        = kENET_QOS_DCB_Mode,
                .mapChannel  = 0x0U,
                .priority    = 0x0U,
                .packetRoute = kENET_QOS_PacketNoQ,
            },
            {
                .mode        = kENET_QOS_AVB_Mode,
                .mapChannel  = 0x1U,
                .priority    = 0x1U,
                .packetRoute = kENET_QOS_PacketNoQ,
            },
            {
                .mode        = kENET_QOS_DCB_Mode,
                .mapChannel  = 0x2U,
                .priority    = 0x2U,
                .packetRoute = kENET_QOS_PacketNoQ,
            },
        },
    };
    config->multiqueueCfg = multiQueue;


    rxpConfig[0] = (enet_qos_rxp_config_t)
    {
        .matchData    = 0x45D9BED3U, /* match DA at frame offset 0 bytes in g_frame[2] */
        .matchEnable  = 0xFFFFFFFFU,
        .acceptFrame  = 1,
        .rejectFrame  = 0,
        .inverseMatch = 0,
        .nextControl  = 0,
        .reserved     = 0,
        .frameOffset  = 0x0U,
        .okIndex      = 0U,
        .dmaChannel   = kENET_QOS_Rxp_DMAChn2, /* Channel 2*/
        .reserved2    = 0,
    };
    rxpConfig[1] = (enet_qos_rxp_config_t)
    {
        .matchData    = 0x00A00081U, /* match frame pattern at offset 12 bytes in g_frame[1] */
        .matchEnable  = 0xFFFFFFFFU,
        .acceptFrame  = 1,
        .rejectFrame  = 0,
        .inverseMatch = 0,
        .nextControl  = 0,
        .reserved     = 0,
        .frameOffset  = 0x3U,
        .okIndex      = 0U,
        .dmaChannel   = kENET_QOS_Rxp_DMAChn1, /* Channel 1*/
        .reserved2    = 0,
    };
    rxpConfig[2] = (enet_qos_rxp_config_t)
    {
        .matchData    = 0xEEDD5533U, /* match frame pattern at offset 16 bytes in g_frame[0] */
        .matchEnable  = 0xFFFFFFFFU,
        .acceptFrame  = 1,
        .rejectFrame  = 0,
        .inverseMatch = 0,
        .nextControl  = 0,
        .reserved     = 0,
        .frameOffset  = 0x4U,
        .okIndex      = 0U,
        .dmaChannel   = kENET_QOS_Rxp_DMAChn0, /* Channel 0*/
        .reserved2    = 0,
    };
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
	return ENET_QOS_MDIOWrite(EXAMPLE_ENET_QOS_BASE, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
	return ENET_QOS_MDIORead(EXAMPLE_ENET_QOS_BASE, phyAddr, regAddr, pData);
}

static void MDIO_Init(uint32_t enet_ipg_freq)
{
	ENET_QOS_SetSMI(EXAMPLE_ENET_QOS_BASE, enet_ipg_freq);

	phy_resource.write = MDIO_Write;
	phy_resource.read = MDIO_Read;
}

static int enet_qos_phy_init(phy_handle_t *phyHandle, bool loopback)
{
    phy_config_t phyConfig = {0};
    status_t status;
    uint32_t count = 0;
    bool link = false;
    bool autonego = false;
    uint32_t enet_ipg_freq;

    phyConfig.phyAddr = EXAMPLE_PHY_ADDR;
    phyConfig.ops = &EXAMPLE_PHY;
    phyConfig.resource = &phy_resource;

    enet_ipg_freq = EXAMPLE_ENET_QOS_CLK_FREQ;

    MDIO_Init(enet_ipg_freq);

    count = 10;
    if (loopback) {
        phyConfig.autoNeg = false;
        phyConfig.speed = kPHY_Speed1000M;
        do
        {
            status = PHY_Init(phyHandle, &phyConfig);
        } while ((status != kStatus_Success) && (--count));
        rtos_assert(status == kStatus_Success, "PHY initialization failed\r\n");
        /* Enable loopback mode */
        PHY_EnableLoopback(phyHandle, kPHY_LocalLoop, kPHY_Speed1000M, true);
        goto init_done;
    }

    phyConfig.autoNeg = true;
    do
    {
        status = PHY_Init(phyHandle, &phyConfig);
        if (status == kStatus_Success)
        {
            /* Wait for auto-negotiation success and link up */
            count = PHY_AUTONEGO_TIMEOUT_COUNT;
            do
            {
                PHY_GetAutoNegotiationStatus(phyHandle, &autonego);
                PHY_GetLinkStatus(phyHandle, &link);
                if (autonego && link)
                {
                    break;
                }
            } while (--count);
            if (!autonego)
            {
                log_info("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
                return kStatus_Fail;
            }
        }
    } while (!(link && autonego));

init_done:
    /* Wait a moment for PHY status to be stable. */
    SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    return kStatus_Success;
}

static int enet_qos_prepare_buffers(enet_qos_buffer_config_t *buffConfig)
{
    uint8_t index = 0;
    uint8_t ringId = 0;
    uint32_t size = 0;
    void *buff = NULL;

    for (ringId = 0; ringId < ENET_QOS_RXQUEUE_USE; ringId++)
    {
        for (index = 0; index < ENET_QOS_RXBD_NUM; index++)
        {
            /* This is for rx buffers, static alloc and dynamic alloc both ok. use as your wish, rx buffer
            malloc should align with cache line to avoid side impact during buffer invalidate.. */
            size = ENET_QOS_RXBUFF_SIZE;

            /* Check cache alignment */
            if ((size & (FSL_FEATURE_L1DCACHE_LINESIZE_BYTE - 1)) != 0)
            {
                size += FSL_FEATURE_L1DCACHE_LINESIZE_BYTE - (size % FSL_FEATURE_L1DCACHE_LINESIZE_BYTE);
            }

            buff = rtos_malloc(size);
            if (buff)
            {
                /* Clean the alloc buffer to avoid there's any dirty data. */
                os_dcache_data_range((uintptr_t)buff, ENET_QOS_RXBUFF_SIZE, OS_CACHE_WB);

                rxbuffer[ringId][index] = (uint32_t)(uintptr_t)buff;
            }
            else
            {
                log_info("Mem Alloc fail\r\n");
                return kStatus_Fail;
            }
        }
    }

    /* prepare the buffer configuration. */
    for (index = 0; index < ENET_QOS_TXQUEUE_USE; index++)
    {
        buffConfig[index] = (enet_qos_buffer_config_t)
        {
            ENET_QOS_RXBD_NUM,
            ENET_QOS_TXBD_NUM,
            &g_txBuffDescrip[index][0],
            &g_txBuffDescrip[index][0],
            &g_txDirty[index][0],
            &g_rxBuffDescrip[index][0],
            &g_rxBuffDescrip[index][ENET_QOS_RXBD_NUM],
            &rxbuffer[index][0],
            ENET_QOS_BuffSizeAlign(ENET_QOS_RXBUFF_SIZE),
            true,
        };
    }

    return kStatus_Success;
}

static void enet_qos_free_buffers(void)
{
    uint8_t index = 0;
    uint8_t ringId = 0;

    for (ringId = 0; ringId < ENET_QOS_RXQUEUE_USE; ringId++)
    {
        for (index = 0; index < ENET_QOS_RXBD_NUM; index++)
        {
            if ( ((void*)(uintptr_t)(rxbuffer[ringId][index])) != NULL)
            {
                rtos_free((void*)(uintptr_t)rxbuffer[ringId][index]);
            }
        }
    }
}

void ethernet_sdk_enet_stats(void *priv)
{
   struct ethernet_ctx *ctx = priv;

   (void)ctx;
}

int ethernet_sdk_enet_run(void *priv, struct event *e)
{
    struct ethernet_ctx *ctx = priv;
    phy_handle_t phyHandle;

    int ret = -1;
    enet_qos_buffer_config_t buffConfig[ENET_QOS_TXQUEUE_USE];
    enet_qos_config_t config;
    enet_qos_ptp_config_t ptpConfig = {0};
    enet_qos_multiqueue_config_t multiQueue;
    enet_qos_rxp_config_t rxpConfig[3];
    uint32_t refClock = ENET_PTP_REF_CLK; /* PTP REF clock. */
    bool link          = false;
    uint32_t ringId    = 0;
    uint32_t testTxNum = 0;
    uint32_t timeout = 0x000FFFFF; /* Just make sure that the test doesn't loop forever if all frames aren't received */

    log_info("\r\n\r\n");
    log_info("####################\r\n");
    log_info("#                  #\r\n");
    log_info("#   enet_qos_app   #\r\n");
    log_info("#                  #\r\n");
    log_info("####################\r\n");

    g_rxIndex  = 0;
    g_rxIndex1 = 0;
    g_rxIndex2 = 0;
    g_txIndex = 0;
    g_txIndex1 = 0;
    g_txIndex2 = 0;
    g_txSuccessFlag = false;
    g_rxSuccessFlag = false;

    /* Register ENET_QOS interrupts */
    os_irq_register(EXAMPLE_ENET_QOS_IRQ, enet_qos_test_irq_handler, NULL, OS_IRQ_PRIO_DEFAULT);

    hardware_ethernet_init();

    ret = enet_qos_prepare_buffers(buffConfig);
    if (ret != kStatus_Success)
    {
        log_info("Unable to prepare buffers: %d\r\n", ret);
        enet_qos_free_buffers();
        os_irq_unregister(EXAMPLE_ENET_QOS_IRQ);
        return ret;
    }

    /* Initialize PHY and wait until auto-negotiation is over. */
    log_info("Wait for PHY init...\r\n");
    ret = enet_qos_phy_init(&phyHandle, ctx->loopback);
    if (ret != kStatus_Success)
    {
        log_info("Unable to initialize phy: %d\r\n", ret);
        enet_qos_free_buffers();
        os_irq_unregister(EXAMPLE_ENET_QOS_IRQ);
        return ret;
    }
    log_info("PHY setup was finalized \r\n");

    enet_qos_prepare_configuration(&config, &ptpConfig, &multiQueue, rxpConfig, &phyHandle, refClock);

    ret = ENET_QOS_Init(EXAMPLE_ENET_QOS_BASE, &config, ctx->mac_addr, 1, refClock);
    if (ret != kStatus_Success)
    {
        log_info("Unable to initialize ENET_QOS: %d\r\n", ret);
        ENET_QOS_Deinit(EXAMPLE_ENET_QOS_BASE);
        enet_qos_free_buffers();
        os_irq_unregister(EXAMPLE_ENET_QOS_IRQ);
        return ret;
    }

    /* Configure rx parser. */
    ret = ENET_QOS_ConfigureRxParser(EXAMPLE_ENET_QOS_BASE, &rxpConfig[0], 3);
    if (ret != kStatus_Success)
    {
        log_info("Unable to configure RX Parser: %d\r\n", ret);
        ENET_QOS_Deinit(EXAMPLE_ENET_QOS_BASE);
        enet_qos_free_buffers();
        os_irq_unregister(EXAMPLE_ENET_QOS_IRQ);
        return ret;
    }

    /* Enable the rx interrupt. */
    ENET_QOS_EnableInterrupts(EXAMPLE_ENET_QOS_BASE, kENET_QOS_DmaRx);

    /* Initialize Descriptor. */
    ret = ENET_QOS_DescriptorInit(EXAMPLE_ENET_QOS_BASE, &config, &buffConfig[0]);
    if (ret != kStatus_Success)
    {
        log_info("Unable to initialize descriptor: %d\r\n", ret);
        ENET_QOS_DisableInterrupts(EXAMPLE_ENET_QOS_BASE, kENET_QOS_DmaRx);
        ENET_QOS_Deinit(EXAMPLE_ENET_QOS_BASE);
        enet_qos_free_buffers();
        os_irq_unregister(EXAMPLE_ENET_QOS_IRQ);
        return ret;
    }

    /* Create the handler. */
    ENET_QOS_CreateHandler(EXAMPLE_ENET_QOS_BASE, &g_handle, &config, &buffConfig[0], ENET_QOS_IntCallback, NULL);

    /* Active TX/RX. */
    ENET_QOS_StartRxTx(EXAMPLE_ENET_QOS_BASE, multiQueue.txQueueUse, multiQueue.rxQueueUse);

    /* Build broadcast for sending and active for receiving. */
    ENET_QOS_BuildFrame(ctx);

    /* Delay some time before executing send and receive operation. */
    SDK_DelayAtLeastUs(1000000ULL, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    log_info("\r\n%d frames ----> will be sent in %d queues, and frames will be received in %d queues.\r\n",
           ENET_QOS_TRANSMIT_DATA_NUM, ENET_QOS_TXQUEUE_USE, ENET_QOS_RXQUEUE_USE);

    /* Start with Ring 2 because ring(queue) 2 has higher priority on tx DMA channel.
    tx Ring N uses DMA channel N and channel N has higher priority than N-1. */
    ringId = 2;
    while (timeout > 0)
    {
        timeout--;
        if (testTxNum < ENET_QOS_TRANSMIT_DATA_NUM)
        {
            if (ctx->loopback)
            {
                link = true;
            }
            else
            {
                /* Send a multicast frame when the PHY is link up. */
                PHY_GetLinkStatus(&phyHandle, &link);
            }

            if (link)
            {
                testTxNum++;
                while (kStatus_Success != ENET_QOS_SendFrame(EXAMPLE_ENET_QOS_BASE, &g_handle, &g_frame[ringId][0],
                                                             ENET_QOS_FRAME_LENGTH, ringId, false, NULL
#if FSL_ENET_QOS_DRIVER_VERSION >= MAKE_VERSION(2, 6, 0)
                                                             , kENET_QOS_TxOffloadDisable
#endif
                                                            ))
                {
                }
                ringId = (ringId + 2) % 3;
            }
        }
        else
        {
            if (ctx->loopback)
            {
                if (g_rxSuccessFlag)
                    break;
            }
            else
            {
                if (g_txSuccessFlag)
                    break;
            }
        }

        if (timeout < 5)
            log_info("%s - Timing out: %d\r\n", __func__, timeout);
    }

    ENET_QOS_DisableInterrupts(EXAMPLE_ENET_QOS_BASE, kENET_QOS_DmaRx);
    ENET_QOS_Deinit(EXAMPLE_ENET_QOS_BASE);
    enet_qos_free_buffers();
    os_irq_unregister(EXAMPLE_ENET_QOS_IRQ);

    log_info("The frames transmitted from the ring 0, 1, 2 is %d, %d, %d, total %d frames!\r\n",
        g_txIndex, g_txIndex1, g_txIndex2, g_txIndex + g_txIndex1 + g_txIndex2);
    log_info("The frames received from the ring 0, 1, 2 is %d, %d, %d, total %d frames!\r\n",
        g_rxIndex, g_rxIndex1, g_rxIndex2, g_rxIndex + g_rxIndex + g_rxIndex2);

    if (ctx->loopback)
    {
        if (g_rxSuccessFlag && g_txSuccessFlag)
        {
            log_info("ENET QOS TXRX Loopback Test PASSED\r\n");
        }
        else
        {
            log_info("ENET QOS TXRX Loopback Test FAILED\r\n");
            return -1;
        }
    }
    else
    {
        if (timeout <=0)
        {
            log_info("ENET QOS TXRX Test FAILED\r\n");
            return -1;
        }
        else
        {
            log_info("ENET QOS TXRX Test Done\r\n");
        }
    }

    return 0;
}

static void *enet_qos_init(void *parameters, bool loopback)
{
    struct industrial_config *cfg = parameters;
    struct ethernet_ctx *ctx;

    ctx = rtos_malloc(sizeof(struct ethernet_ctx));
    if (!ctx)
    {
        log_err("Memory allocation error\n");
        goto exit;
    }

    memset(ctx, 0, sizeof(struct ethernet_ctx));

    ctx->event_send = cfg->event_send;
    ctx->event_data = cfg->event_data;
    ctx->loopback = loopback;

    log_info("%s\n", __func__);

exit:
    return ctx;
}

void *ethernet_sdk_enet_init(void *parameters)
{
    return enet_qos_init(parameters, false);
}

void *ethernet_sdk_enet_loopback_init(void *parameters)
{
    return enet_qos_init(parameters, true);
}

void ethernet_sdk_enet_exit(void *priv)
{
    struct ethernet_ctx *ctx = priv;

    rtos_free(ctx);

    log_info("end\n");
}

inline void ethernet_sdk_enet_loopback_exit(void *priv)
{
    ethernet_sdk_enet_exit(priv);
}

inline int ethernet_sdk_enet_loopback_run(void *priv, struct event *e)
{
    return ethernet_sdk_enet_run(priv, e);
}

inline void ethernet_sdk_enet_loopback_stats(void *priv)
{
    ethernet_sdk_enet_stats(priv);
}
