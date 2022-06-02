/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_board.h"
#include "fsl_flexcan.h"
#include "hardware_flexcan.h"
#include "hlog.h"
#include "industrial.h"

#include "os/irq.h"
#include "os/stdlib.h"
#include "os/string.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_CAN                FLEXCAN1
#define EXAMPLE_FLEXCAN_IRQn       CAN_FD1_IRQn
#define EXAMPLE_CAN_CLK_FREQ                                                                    \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootFlexCan1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootFlexCan1)))
/* Set USE_IMPROVED_TIMING_CONFIG macro to use api to calculates the improved CAN / CAN FD timing values. */
#define USE_IMPROVED_TIMING_CONFIG (1U)

/* Considering that the first valid MB must be used as Reserved TX MB for ERR005641. */
#define RX_QUEUE_BUFFER_BASE  (1U)
#define RX_QUEUE_BUFFER_SIZE  (4U)
#define RX_MESSAGE_BUFFER_NUM (9)
#define TX_MESSAGE_BUFFER_NUM (8)

/* Define USE_CANFD to no-zero value to evaluate CANFD mode, define to 0 to use classical CAN mode*/
#define USE_CANFD (1)
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
#define DLC         (8)
#define BYTES_IN_MB kFLEXCAN_8BperMB
#define DWORD_IN_MB (2)
/* Rx queue end Message Buffer index. */
#define RX_QUEUE_BUFFER_END_1 (RX_QUEUE_BUFFER_BASE + RX_QUEUE_BUFFER_SIZE - 1U)
#define RX_QUEUE_BUFFER_END_2 (RX_QUEUE_BUFFER_BASE + RX_QUEUE_BUFFER_SIZE * 2U - 1U)
/* Tx MB ID. */
#define TX_MB_ID 0x321UL
/* RX MB individual mask, which will make FLEXCAN to check only 1~8 bits of the ID of the received message. */
#define RX_MB_ID_MASK 0xFFUL
/* RX MB ID after mask. */
#define RX_MB_ID_AFTER_MASK (RX_MB_ID_MASK & TX_MB_ID)

struct can_ctx {
	void (*event_send)(void *, uint8_t);
	void *event_data;

	flexcan_handle_t flexcanHandle;
	volatile bool txComplete;
	volatile bool rxComplete;
	volatile bool wakenUp;

	flexcan_mb_transfer_t txXfer, rxXfer;

#if (defined(USE_CANFD) && USE_CANFD)
	flexcan_fd_frame_t frame;
	flexcan_fd_frame_t rxFrame[RX_QUEUE_BUFFER_SIZE * 2];
#else
	flexcan_frame_t frame;
	flexcan_frame_t rxFrame[RX_QUEUE_BUFFER_SIZE * 2];
#endif

	uint32_t txIdentifier;
	uint32_t rxIdentifier;

	uint8_t node_type;
	uint8_t test_type;

	volatile status_t rxStatus;
	volatile uint32_t rxQueueNum;
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief User read message buffer function
 */
#if (defined(USE_CANFD) && USE_CANFD)
static status_t User_ReadRxMb(CAN_Type *base, uint8_t mbIdx, flexcan_fd_frame_t *pRxFrame)
#else
static status_t User_ReadRxMb(CAN_Type *base, uint8_t mbIdx, flexcan_frame_t *pRxFrame)
#endif
{
    /* Assertion. */
    assert(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    assert(NULL != pRxFrame);

    status_t status;
    uint32_t cs_temp;
    uint8_t rx_code;
    uint32_t can_id = 0;
#if (defined(USE_CANFD) && USE_CANFD)
    uint8_t cnt = 0;
    uint32_t dataSize;
    dataSize                  = (base->FDCTRL & CAN_FDCTRL_MBDSR0_MASK) >> CAN_FDCTRL_MBDSR0_SHIFT;
    uint8_t payload_dword     = 1;
    volatile uint32_t *mbAddr = &(base->MB[0].CS);
    uint32_t offset = 0U;

    switch (dataSize)
    {
        case (uint32_t)kFLEXCAN_8BperMB:
            offset = (((uint32_t)mbIdx / 32U) * 512U + ((uint32_t)mbIdx % 32U) * 16U);
            break;
        case (uint32_t)kFLEXCAN_16BperMB:
            offset = (((uint32_t)mbIdx / 21U) * 512U + ((uint32_t)mbIdx % 21U) * 24U);
            break;
        case (uint32_t)kFLEXCAN_32BperMB:
            offset = (((uint32_t)mbIdx / 12U) * 512U + ((uint32_t)mbIdx % 12U) * 40U);
            break;
        case (uint32_t)kFLEXCAN_64BperMB:
            offset = (((uint32_t)mbIdx / 7U) * 512U + ((uint32_t)mbIdx % 7U) * 72U);
            break;
        default:
            /* All the cases have been listed above, the default clause should not be reached. */
            assert(false);
            break;
    }
    /* To get the dword aligned offset, need to divide by 4. */
    offset = offset / 4U;

    /* Read CS field of Rx Message Buffer to lock Message Buffer. */
    cs_temp = mbAddr[offset];
    can_id  = mbAddr[offset + 1U];
#else
    /* Read CS field of Rx Message Buffer to lock Message Buffer. */
    cs_temp = base->MB[mbIdx].CS;
    can_id  = base->MB[mbIdx].ID;
#endif
    /* Get Rx Message Buffer Code field. */
    rx_code = (uint8_t)((cs_temp & CAN_CS_CODE_MASK) >> CAN_CS_CODE_SHIFT);

    /* Check to see if Rx Message Buffer is full or overrun. */
    if ((0x2 == rx_code) || (0x6 == rx_code))
    {
        /* Store Message ID. */
        pRxFrame->id = can_id & (CAN_ID_EXT_MASK | CAN_ID_STD_MASK);

        /* Get the message ID and format. */
        pRxFrame->format = (cs_temp & CAN_CS_IDE_MASK) != 0U ? (uint8_t)kFLEXCAN_FrameFormatExtend :
                                                               (uint8_t)kFLEXCAN_FrameFormatStandard;

        /* Get the message type. */
        pRxFrame->type =
            (cs_temp & CAN_CS_RTR_MASK) != 0U ? (uint8_t)kFLEXCAN_FrameTypeRemote : (uint8_t)kFLEXCAN_FrameTypeData;

        /* Get the message length. */
        pRxFrame->length = (uint8_t)((cs_temp & CAN_CS_DLC_MASK) >> CAN_CS_DLC_SHIFT);

        /* Get the time stamp. */
        pRxFrame->timestamp = (uint16_t)((cs_temp & CAN_CS_TIME_STAMP_MASK) >> CAN_CS_TIME_STAMP_SHIFT);

#if (defined(USE_CANFD) && USE_CANFD)
        /* Calculate the DWORD number, dataSize 0/1/2/3 corresponds to 8/16/32/64
           Bytes payload. */
        for (cnt = 0; cnt < (dataSize + 1U); cnt++)
        {
            payload_dword *= 2U;
        }

        /* Store Message Payload. */
        for (cnt = 0; cnt < payload_dword; cnt++)
        {
            pRxFrame->dataWord[cnt] = mbAddr[offset + 2U + cnt];
        }

        /* Restore original Rx ID value*/
        mbAddr[offset + 1U] = FLEXCAN_ID_STD(RX_MB_ID_AFTER_MASK);
#else
        /* Store Message Payload. */
        pRxFrame->dataWord0 = base->MB[mbIdx].WORD0;
        pRxFrame->dataWord1 = base->MB[mbIdx].WORD1;
        /* Restore original Rx value*/
        base->MB[mbIdx].ID = FLEXCAN_ID_STD(RX_MB_ID_AFTER_MASK);
#endif
        /* Read free-running timer to unlock Rx Message Buffer. */
        (void)base->TIMER;

        if (0x2 == rx_code)
        {
            status = kStatus_Success;
        }
        else
        {
            status = kStatus_FLEXCAN_RxOverflow;
        }
    }
    else
    {
        /* Read free-running timer to unlock Rx Message Buffer. */
        (void)base->TIMER;

        status = kStatus_Fail;
    }

    return status;
}

/*!
 * @brief User FlexCAN IRQ handler function - PingPong Example
 */
static void User_TransferHandleIRQ(CAN_Type *base, struct can_ctx *ctx)
{
    status_t status;
    uint32_t EsrStatus;
    uint32_t i;

    do
    {
        /* Get Current FlexCAN Module Error and Status. */
        EsrStatus = FLEXCAN_GetStatusFlags(base);

        /* To handle FlexCAN Error and Status Interrupt first. */
        if (0U != (EsrStatus & ((uint32_t)kFLEXCAN_TxWarningIntFlag | (uint32_t)kFLEXCAN_RxWarningIntFlag |
                                (uint32_t)kFLEXCAN_BusOffIntFlag | (uint32_t)kFLEXCAN_ErrorIntFlag)))
        {
            ctx->rxStatus = kStatus_FLEXCAN_ErrorStatus;
            /* Clear FlexCAN Error and Status Interrupt. */
            FLEXCAN_ClearStatusFlags(base, (uint32_t)kFLEXCAN_TxWarningIntFlag | (uint32_t)kFLEXCAN_RxWarningIntFlag |
                                               (uint32_t)kFLEXCAN_BusOffIntFlag | (uint32_t)kFLEXCAN_ErrorIntFlag);
        }
        else if (0U != (EsrStatus & (uint32_t)kFLEXCAN_WakeUpIntFlag))
        {
            ctx->rxStatus = kStatus_FLEXCAN_WakeUp;
            /* Clear FlexCAN Wake Up Interrupt. */
            FLEXCAN_ClearStatusFlags(base, (uint32_t)kFLEXCAN_WakeUpIntFlag);
        }
        else
        {
            /* Handle real data transfer. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
            if (0U != FLEXCAN_GetMbStatusFlags(base, (uint64_t)1U << RX_QUEUE_BUFFER_END_1))
#else
            if (0U != FLEXCAN_GetMbStatusFlags(base, (uint32_t)1U << RX_QUEUE_BUFFER_END_1))
#endif
            {
                /* Queue 1 end Message Buffer interrupt. */
                ctx->rxQueueNum = 1U;
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                {
                    /* Default receive message ID is 0x321, and after individual mask 0xFF, can match the Rx queue MB ID
                       0x21. Change queue 1 MBs individual ID mask to 0x3FF, will make it can't match the Rx queue MB
                       ID, so queue 1 will ignore the next CAN/CANFD messages and the queue 2 will receive the messages.
                     */
                    FLEXCAN_SetRxIndividualMask(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i,
                                                FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK | TX_MB_ID, 0, 0));
                    (void)User_ReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &ctx->rxFrame[i]);
                    /* Clear queue 1 Message Buffer receive status. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
                    FLEXCAN_ClearMbStatusFlags(base, (uint64_t)1U << (RX_QUEUE_BUFFER_BASE + i));
#else
                    FLEXCAN_ClearMbStatusFlags(base, (uint32_t)1U << (RX_QUEUE_BUFFER_BASE + i));
#endif
                }
            }
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
            else if (0U != FLEXCAN_GetMbStatusFlags(base, (uint64_t)1U << RX_QUEUE_BUFFER_END_2))
#else
            else if (0U != FLEXCAN_GetMbStatusFlags(base, (uint32_t)1U << RX_QUEUE_BUFFER_END_2))
#endif
            {
                /* Queue 2 end Message Buffer interrupt. */
                ctx->rxQueueNum = 2U;
                /* Restore queue 1 ID mask to make it can receive the next CAN/CANFD messages again. */
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                    FLEXCAN_SetRxIndividualMask(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i,
                                                FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK, 0, 0));
                for (; i < (RX_QUEUE_BUFFER_SIZE * 2U); i++)
                {
                    status = User_ReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &ctx->rxFrame[i]);

                    /* Clear queue 2 Message Buffer receive status. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
                    FLEXCAN_ClearMbStatusFlags(base, (uint64_t)1U << (RX_QUEUE_BUFFER_BASE + i));
#else
                    FLEXCAN_ClearMbStatusFlags(base, (uint32_t)1U << (RX_QUEUE_BUFFER_BASE + i));
#endif
                    /* Due to enable the queue feature, the rx overflow may only occur in the last matched MB
                       (RX_QUEUE_BUFFER_END_2). */
                    if (((RX_QUEUE_BUFFER_BASE + i) == RX_QUEUE_BUFFER_END_2) && (status == kStatus_FLEXCAN_RxOverflow))
                        ctx->rxStatus = status;
                }
            }
        }
    }
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
    while (
        (0U != FLEXCAN_GetMbStatusFlags(
                   base, ((uint64_t)1U << RX_QUEUE_BUFFER_END_1) | ((uint64_t)1U << RX_QUEUE_BUFFER_END_2))) ||
        (0U != (FLEXCAN_GetStatusFlags(base) & ((uint32_t)kFLEXCAN_TxWarningIntFlag |
                                                (uint32_t)kFLEXCAN_RxWarningIntFlag | (uint32_t)kFLEXCAN_BusOffIntFlag |
                                                (uint32_t)kFLEXCAN_ErrorIntFlag | (uint32_t)kFLEXCAN_WakeUpIntFlag))));
#else
    while (
        (0U != FLEXCAN_GetMbStatusFlags(
                   base, ((uint32_t)1U << RX_QUEUE_BUFFER_END_1) | ((uint32_t)1U << RX_QUEUE_BUFFER_END_2))) ||
        (0U != (FLEXCAN_GetStatusFlags(base) & ((uint32_t)kFLEXCAN_TxWarningIntFlag |
                                                (uint32_t)kFLEXCAN_RxWarningIntFlag | (uint32_t)kFLEXCAN_BusOffIntFlag |
                                                (uint32_t)kFLEXCAN_ErrorIntFlag | (uint32_t)kFLEXCAN_WakeUpIntFlag))));
#endif
}


/*!
 * @brief User FlexCAN IRQ handler function - Interrupt Example
 */
static FLEXCAN_CALLBACK(flexcan_callback)
{
    struct can_ctx *ctx = userData;

    switch (status)
    {
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MESSAGE_BUFFER_NUM == result)
            {
                ctx->rxComplete = true;
            }
            break;

        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                ctx->txComplete = true;
            }
            break;

        case kStatus_FLEXCAN_WakeUp:
            ctx->wakenUp = true;
            break;

        default:
            break;
    }
}

/*!
 * @brief User FlexCAN IRQ handler function - Loopback Example
 */
static void EXAMPLE_FLEXCAN_IRQHandler(void *data)
{
    struct can_ctx *ctx = data;

#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
    uint64_t flag = 1U;
#else
    uint32_t flag = 1U;
#endif
    /* If new data arrived. */
    if (0U != FLEXCAN_GetMbStatusFlags(EXAMPLE_CAN, flag << RX_MESSAGE_BUFFER_NUM))
    {
        FLEXCAN_ClearMbStatusFlags(EXAMPLE_CAN, flag << RX_MESSAGE_BUFFER_NUM);
#if (defined(USE_CANFD) && USE_CANFD)
        (void)FLEXCAN_ReadFDRxMb(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &ctx->frame);
#else
        (void)FLEXCAN_ReadRxMb(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &ctx->frame);
#endif
        ctx->rxComplete = true;
    }
}

void test_flexcan_irq_handler(void *data)
{
    struct can_ctx *ctx = data;

    switch (ctx->test_type)
    {
        case 2:
            FLEXCAN_TransferHandleIRQ(EXAMPLE_CAN, &ctx->flexcanHandle);
            break;

        case 3:
            User_TransferHandleIRQ(EXAMPLE_CAN, ctx);
            break;

        default:
            break;
    }

}

static void test_flexcan_setup(struct can_ctx *ctx)
{
    uint8_t test_type = ctx->test_type;
    uint8_t node_type = ctx->node_type;
    flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t mbConfig;
    uint32_t i;

    log_info("enter\n");

    /* Select mailbox ID. */
    if ((node_type == 'A') || (node_type == 'a'))
    {
        ctx->txIdentifier = 0x321;
        ctx->rxIdentifier = 0x123;
    } else {
        ctx->txIdentifier = 0x123;
        ctx->rxIdentifier = 0x321;
    }

    /* Get FlexCAN module default Configuration. */
    /*
     * flexcanConfig.clkSrc                 = kFLEXCAN_ClkSrc0;
     * flexcanConfig.baudRate               = 1000000U;
     * flexcanConfig.baudRateFD             = 2000000U;
     * flexcanConfig.maxMbNum               = 16;
     * flexcanConfig.enableLoopBack         = false;
     * flexcanConfig.enableSelfWakeup       = false;
     * flexcanConfig.enableIndividMask      = false;
     * flexcanConfig.disableSelfReception   = false;
     * flexcanConfig.enableListenOnlyMode   = false;
     * flexcanConfig.enableDoze             = false;
     */
    FLEXCAN_GetDefaultConfig(&flexcanConfig);

    if (test_type == 1)
    {
        flexcanConfig.enableLoopBack = true;
    }
    else if (test_type == 3)
    {
        /* Enable Rx Individual Mask and Queue feature. */
        flexcanConfig.enableIndividMask = true;
    }

#if defined(EXAMPLE_CAN_CLK_SOURCE)
    flexcanConfig.clkSrc = EXAMPLE_CAN_CLK_SOURCE;
#endif

/* If special quantum setting is needed, set the timing parameters. */
#if (defined(SET_CAN_QUANTUM) && SET_CAN_QUANTUM)
    flexcanConfig.timingConfig.phaseSeg1 = PSEG1;
    flexcanConfig.timingConfig.phaseSeg2 = PSEG2;
    flexcanConfig.timingConfig.propSeg   = PROPSEG;
#if (defined(FSL_FEATURE_FLEXCAN_HAS_FLEXIBLE_DATA_RATE) && FSL_FEATURE_FLEXCAN_HAS_FLEXIBLE_DATA_RATE)
    flexcanConfig.timingConfig.fphaseSeg1 = FPSEG1;
    flexcanConfig.timingConfig.fphaseSeg2 = FPSEG2;
    flexcanConfig.timingConfig.fpropSeg   = FPROPSEG;
#endif
#endif

/* Use the FLEXCAN API to automatically get the ideal bit timing configuration. */
#if (defined(USE_IMPROVED_TIMING_CONFIG) && USE_IMPROVED_TIMING_CONFIG)
    flexcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(flexcan_timing_config_t));
#if (defined(USE_CANFD) && USE_CANFD)
    if (FLEXCAN_FDCalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.baudRate, flexcanConfig.baudRateFD,
                                                EXAMPLE_CAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    }
    else
    {
        log_info("No found Improved Timing Configuration. Just used default configuration\n\n");
    }
#else
    if (FLEXCAN_CalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.baudRate, EXAMPLE_CAN_CLK_FREQ,
                                              &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    }
    else
    {
        log_info("No found Improved Timing Configuration. Just used default configuration\n\n");
    }
#endif
#endif

    log_info("Init CAN with clock freq %d", EXAMPLE_CAN_CLK_FREQ);
#if (defined(USE_CANFD) && USE_CANFD)
    FLEXCAN_FDInit(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ, BYTES_IN_MB, true);
#else
    FLEXCAN_Init(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ);
#endif

    if (test_type == 1)
    {
        /* Setup Rx Message Buffer. */
        mbConfig.format = kFLEXCAN_FrameFormatStandard;
        mbConfig.type   = kFLEXCAN_FrameTypeData;
        mbConfig.id     = FLEXCAN_ID_STD(ctx->txIdentifier);
#if (defined(USE_CANFD) && USE_CANFD)
        FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
#else
        FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
#endif

        /* Setup Tx Message Buffer. */
#if (defined(USE_CANFD) && USE_CANFD)
        FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
#else
        FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
#endif

        /* Enable Rx Message Buffer interrupt. */
        os_irq_register(EXAMPLE_FLEXCAN_IRQn, EXAMPLE_FLEXCAN_IRQHandler, ctx, 0xe1);

        /* Enable receive interrupt for Rx queue 1 & 2 end Message Buffer. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
        FLEXCAN_EnableMbInterrupts(EXAMPLE_CAN, (uint64_t)1U << RX_MESSAGE_BUFFER_NUM);
#else
        FLEXCAN_EnableMbInterrupts(EXAMPLE_CAN, (uint32_t)1U << RX_MESSAGE_BUFFER_NUM);
#endif
        os_irq_enable(EXAMPLE_FLEXCAN_IRQn);
    }
    else if (test_type == 2)
    {

        /* Create FlexCAN handle structure and set call back function. */
        FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &ctx->flexcanHandle, flexcan_callback, ctx);

        os_irq_register(EXAMPLE_FLEXCAN_IRQn, test_flexcan_irq_handler, ctx, 0xe1);
        os_irq_enable(EXAMPLE_FLEXCAN_IRQn);

        /* Set Rx Masking mechanism. */
        FLEXCAN_SetRxMbGlobalMask(EXAMPLE_CAN, FLEXCAN_RX_MB_STD_MASK(ctx->rxIdentifier, 0, 0));

        /* Setup Rx Message Buffer. */
        mbConfig.format = kFLEXCAN_FrameFormatStandard;
        mbConfig.type   = kFLEXCAN_FrameTypeData;
        mbConfig.id     = FLEXCAN_ID_STD(ctx->rxIdentifier);
#if (defined(USE_CANFD) && USE_CANFD)
        FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
#else
        FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
#endif

        /* Setup Tx Message Buffer. */
#if (defined(USE_CANFD) && USE_CANFD)
        FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
#else
        FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
#endif
        if ((node_type == 'A') || (node_type == 'a'))
        {
            log_info("Press any key to trigger one-shot transmission\n\n");
            ctx->frame.dataByte0 = 0;
        }
        else
        {
            log_info("Start to Wait data from Node A\n\n");
        }
    }
    else if (test_type == 3)
    {
        if ((node_type == 'A') || (node_type == 'a'))
        {
            /* Setup Tx Message Buffer. */
#if (defined(USE_CANFD) && USE_CANFD)
            FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
#else
            FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
#endif
            ctx->frame.dataByte0 = 0;
            ctx->frame.dataByte1 = 0x55;
        }
        else
        {
            FLEXCAN_EnableInterrupts(EXAMPLE_CAN, (uint32_t)kFLEXCAN_BusOffInterruptEnable |
                                                  (uint32_t)kFLEXCAN_ErrorInterruptEnable |
                                                  (uint32_t)kFLEXCAN_RxWarningInterruptEnable);
            /* Register interrupt. */
            os_irq_register(EXAMPLE_FLEXCAN_IRQn, test_flexcan_irq_handler, ctx, 0xe1);
            /* Setup Rx Message Buffer. */
            mbConfig.format = kFLEXCAN_FrameFormatStandard;
            mbConfig.type   = kFLEXCAN_FrameTypeData;
            mbConfig.id     = FLEXCAN_ID_STD(RX_MB_ID_AFTER_MASK);

            for (i = 0U; i < RX_QUEUE_BUFFER_SIZE * 2U; i++)
            {
                /* Setup Rx individual ID mask. */
                FLEXCAN_SetRxIndividualMask(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i,
                                        FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK, 0, 0));
#if (defined(USE_CANFD) && USE_CANFD)
                FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &mbConfig, true);
#else
                FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &mbConfig, true);
#endif
            }
            /* Enable receive interrupt for Rx queue 1 & 2 end Message Buffer. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
            FLEXCAN_EnableMbInterrupts(EXAMPLE_CAN, (uint64_t)1U << RX_QUEUE_BUFFER_END_1);
            FLEXCAN_EnableMbInterrupts(EXAMPLE_CAN, (uint64_t)1U << RX_QUEUE_BUFFER_END_2);
#else
            FLEXCAN_EnableMbInterrupts(EXAMPLE_CAN, (uint32_t)1U << RX_QUEUE_BUFFER_END_1);
            FLEXCAN_EnableMbInterrupts(EXAMPLE_CAN, (uint32_t)1U << RX_QUEUE_BUFFER_END_2);
#endif
            os_irq_enable(EXAMPLE_FLEXCAN_IRQn);

            log_info("Start to Wait data from Node A\n\n");
        }
    }
    log_info("end\n");
}

static void test_flexcan_transmit(struct can_ctx *ctx)
{
    uint8_t test_type = ctx->test_type;
    uint8_t node_type = ctx->node_type;
    static uint32_t TxCount = 1;
    uint32_t i;

    log_info("enter\n");

    if ((node_type == 'A') || (node_type == 'a') || test_type == 1)
    {
        ctx->frame.id     = FLEXCAN_ID_STD(ctx->txIdentifier);
        ctx->frame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
        ctx->frame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
        ctx->frame.length = (uint8_t)DLC;
#if (defined(USE_CANFD) && USE_CANFD)
        ctx->frame.brs = (uint8_t)1U;
#endif
        if (test_type == 1)
        {
#if (defined(USE_CANFD) && USE_CANFD)
            for (i = 0; i < DWORD_IN_MB; i++)
            {
               ctx->frame.dataWord[i] = i;
            }
#else
            ctx->frame.dataWord0 = CAN_WORD0_DATA_BYTE_0(0x11) | CAN_WORD0_DATA_BYTE_1(0x22) | CAN_WORD0_DATA_BYTE_2(0x33) |
                              CAN_WORD0_DATA_BYTE_3(0x44);
            ctx->frame.dataWord1 = CAN_WORD1_DATA_BYTE_4(0x55) | CAN_WORD1_DATA_BYTE_5(0x66) | CAN_WORD1_DATA_BYTE_6(0x77) |
                              CAN_WORD1_DATA_BYTE_7(0x88);
#endif

            log_info("Send message from MB%d to MB%d\n", TX_MESSAGE_BUFFER_NUM, RX_MESSAGE_BUFFER_NUM);
#if (defined(USE_CANFD) && USE_CANFD)
            for (i = 0; i < DWORD_IN_MB; i++)
            {
                log_info("tx word%d = 0x%x\n", i, ctx->frame.dataWord[i]);
            }
#else
            log_info("tx word0 = 0x%x\n", ctx->frame.dataWord0);
            log_info("tx word1 = 0x%x\n", ctx->frame.dataWord1);
#endif

            /* Send data through Tx Message Buffer using polling function. */
#if (defined(USE_CANFD) && USE_CANFD)
            (void)FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->frame);
#else
            (void)FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->frame);
#endif

            /* Waiting for Message receive finish. */
            while (!ctx->rxComplete)
            {
            }

            log_info("\nReceived message from MB%d\n", RX_MESSAGE_BUFFER_NUM);
#if (defined(USE_CANFD) && USE_CANFD)
            for (i = 0; i < DWORD_IN_MB; i++)
            {
                log_info("rx word%d = 0x%x\n", i, ctx->frame.dataWord[i]);
            }
#else
            log_info("rx word0 = 0x%x\n", ctx->frame.dataWord0);
            log_info("rx word1 = 0x%x\n", ctx->frame.dataWord1);
#endif

            /* Stop FlexCAN Send & Receive. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
            FLEXCAN_DisableMbInterrupts(EXAMPLE_CAN, (uint64_t)1U << RX_MESSAGE_BUFFER_NUM);
#else
            FLEXCAN_DisableMbInterrupts(EXAMPLE_CAN, (uint32_t)1U << RX_MESSAGE_BUFFER_NUM);
#endif

            log_info("\n==FlexCAN loopback functional example -- Finish.==\n");

        } else if (test_type == 2) {
            GETCHAR();

            ctx->txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            ctx->txXfer.framefd = &ctx->frame;
            (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
#else
            ctx->txXfer.ctx->frame = &ctx->frame;
            (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
#endif

            log_info("Transmission began\n");
            while (!ctx->txComplete)
            {
            };
            ctx->txComplete = false;

            log_info("Transmission sent\n");
            /* Start receive data through Rx Message Buffer. */
            ctx->rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            ctx->rxXfer.framefd = &ctx->frame;
            (void)FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
#else
            ctx->rxXfer.ctx->frame = &ctx->frame;
            (void)FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
#endif

            /* Wait until Rx MB full. */
            while (!ctx->rxComplete)
            {
            };
            ctx->rxComplete = false;

            log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->frame.id >> CAN_ID_STD_SHIFT,
                     ctx->frame.dataByte0, ctx->frame.timestamp);
            ctx->frame.dataByte0++;
            ctx->frame.dataByte1 = 0x55;

            log_info("\n==FlexCAN interrupt functional example -- Finish.==\n");
        } else if (test_type == 3) {
            uint8_t index  = 0;
            uint32_t times = 0;
            log_info("Please input the number of CAN/CANFD messages to be send and end with enter.\n");
            while (index != 0x0D)
            {
                index = GETCHAR();
                if ((index >= '0') && (index <= '9'))
                {
                    (void)PUTCHAR(index);
                    times = times * 10 + (index - 0x30U);
                }
            }
            log_info("\n");

            for (i = 1; i <= times; i++)
            {
#if (defined(USE_CANFD) && USE_CANFD)
                (void)FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->frame);
#else
                (void)FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->frame);
#endif
                /* Wait for 200ms after every 2 RX_QUEUE_BUFFER_SIZE transmissions. */
                if ((TxCount % (RX_QUEUE_BUFFER_SIZE * 2U)) == 0U)
                    SDK_DelayAtLeastUs(200000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

                ctx->frame.dataByte0++;
                TxCount++;
            }
            log_info("Transmission done.\n\n");
            log_info("\n==FlexCAN PingPong functional example -- Finish.==\n");
        }
    }
    else
    {
        if (test_type == 2)
        {
            /* Before this , should first make node B enter STOP mode after FlexCAN
             * initialized with enableSelfWakeup=true and Rx MB configured, then A
             * sends ctx->frame N which wakes up node B. A will continue to send ctx->frame N
             * since no acknowledgement, then B received the second ctx->frame N(In the
             * application it seems that B received the ctx->frame that woke it up which
             * is not expected as stated in the reference manual, but actually the
             * output in the terminal B received is the same second ctx->frame N). */
            if (ctx->wakenUp)
            {
                log_info("B has been waken up!\n\n");
            }

            /* Start receive data through Rx Message Buffer. */
            ctx->rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            ctx->rxXfer.framefd = &ctx->frame;
            (void)FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
#else
            ctx->rxXfer.ctx->frame = &ctx->frame;
            (void)FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
#endif

            /* Wait until Rx receive full. */
            while (!ctx->rxComplete)
            {
            };
            ctx->rxComplete = false;

            log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->frame.id >> CAN_ID_STD_SHIFT,
                 ctx->frame.dataByte0, ctx->frame.timestamp);

            ctx->frame.id     = FLEXCAN_ID_STD(ctx->txIdentifier);
            ctx->txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            ctx->frame.brs      = 1;
            ctx->txXfer.framefd = &ctx->frame;
            (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
#else
            ctx->txXfer.ctx->frame = &ctx->frame;
            (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
#endif

            while (!ctx->txComplete)
            {
            };
            ctx->txComplete = false;

            log_info("\n==FlexCAN interrupt functional example -- Finish.==\n");
        } else if (test_type == 3) {
            while (true)
            {
                /* Wait until Rx queue 1 full. */
                while (ctx->rxQueueNum != 1U)
                {
                };
                ctx->rxQueueNum = 0;
                log_info("Read Rx MB from Queue 1.\n");
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                {
                    log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->rxFrame[i].id >> CAN_ID_STD_SHIFT,
                             ctx->rxFrame[i].dataByte0, ctx->rxFrame[i].timestamp);
                }
                /* Wait until Rx queue 2 full. */
                while (ctx->rxQueueNum != 2U)
                {
                };
                ctx->rxQueueNum = 0;
                log_info("Read Rx MB from Queue 2.\n");
                for (; i < (RX_QUEUE_BUFFER_SIZE * 2U); i++)
                {
                    log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->rxFrame[i].id >> CAN_ID_STD_SHIFT,
                             ctx->rxFrame[i].dataByte0, ctx->rxFrame[i].timestamp);
                }
                if (ctx->rxStatus == kStatus_FLEXCAN_RxOverflow)
                {
                    ctx->rxStatus = 0;
                    log_info("The data in the last MB %d in the queue 2 is overwritten\n", RX_QUEUE_BUFFER_END_2);
                }
                log_info("Wait Node A to trigger the next 8 messages!\n\n");
            }
            log_info("\n==FlexCAN PingPong functional example -- Finish.==\n");
        }
    }
    os_irq_unregister(EXAMPLE_FLEXCAN_IRQn);

    log_info("end\n");
}

void can_stats(void *priv)
{
	struct can_ctx *ctx = priv;

	(void)ctx;
	log_info("not implemented\n");
}

int can_run(void *priv, struct event *e)
{
	struct can_ctx *ctx = priv;

	log_info("FLEXCAN PingPong Buffer - node %C\n", ctx->node_type);
	log_info("    Message format: Standard (11 bit id)\n");
	log_info("    Node B Message buffer %d to %d used as Rx queue 1.\n", RX_QUEUE_BUFFER_BASE, RX_QUEUE_BUFFER_END_1);
	log_info("    Node B Message buffer %d to %d used as Rx queue 2.\n", RX_QUEUE_BUFFER_END_1 + 1U, RX_QUEUE_BUFFER_END_2);
	log_info("    Node A Message buffer %d used as Tx.\n", TX_MESSAGE_BUFFER_NUM);

	test_flexcan_transmit(ctx);

	return 0;
}

static void *can_init(void *parameters, uint32_t test_type)
{
	struct industrial_config *cfg = parameters;
	struct can_ctx *ctx;

	ctx = os_malloc(sizeof(struct can_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");

		goto exit;
	}

	memset(ctx, 0, sizeof(struct can_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;

	log_info("enter\n");

	hardware_flexcan_init();

	/* FIXME - Get node_type from ctrl app parameters */
	ctx->node_type = 'b';
	log_info("node %c\n", ctx->node_type);
	log_info("test type %d\n", test_type);

	ctx->test_type = test_type;

	test_flexcan_setup(ctx);
exit:
	return ctx;
}

void *can_init_loopback(void *parameters)
{
	log_info("********* FLEXCAN Loopback EXAMPLE *********\n");
	log_info("    Message format: Standard (11 bit id)\n");
	log_info("    Loopback Mode: Enabled\n");
	log_info("*********************************************\n\n");

	return can_init(parameters, 1);
}

void *can_init_interrupt(void *parameters)
{
	log_info("********* FLEXCAN Interrupt EXAMPLE *********\n");
	log_info("    Message format: Standard (11 bit id)\n");
	log_info("    Message buffer %d used for Rx.\n", RX_MESSAGE_BUFFER_NUM);
	log_info("    Message buffer %d used for Tx.\n", TX_MESSAGE_BUFFER_NUM);
	log_info("    Interrupt Mode: Enabled\n");
	log_info("    Operation Mode: TX and RX --> Normal\n");
	log_info("*********************************************\n\n");

	return can_init(parameters, 2);
}

void *can_init_pingpong(void *parameters)
{
	log_info("********* FLEXCAN PingPong Buffer Example *********\n");
	log_info("    Message format: Standard (11 bit id)\n");
	log_info("    Node B Message buffer %d to %d used as Rx queue 1.\n", RX_QUEUE_BUFFER_BASE, RX_QUEUE_BUFFER_END_1);
	log_info("    Node B Message buffer %d to %d used as Rx queue 2.\n", RX_QUEUE_BUFFER_END_1 + 1U, RX_QUEUE_BUFFER_END_2);
	log_info("    Node A Message buffer %d used as Tx.\n", TX_MESSAGE_BUFFER_NUM);

	return can_init(parameters, 3);
}

void can_exit(void *priv)
{
	struct can_ctx *ctx = priv;

	os_free(ctx);

	log_info("end\n");
}
