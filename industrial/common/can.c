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
#include "os/unistd.h"

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

enum {
	CAN_NODE_A = 0,
	CAN_NODE_B,
};

struct can_ctx {
	void (*event_send)(void *, uint8_t, uint8_t);
	void *event_data;

	flexcan_handle_t flexcanHandle;
	volatile bool txComplete;
	volatile bool rxComplete;
	volatile bool wakenUp;

	flexcan_mb_transfer_t txXfer, rxXfer;

	union {
		flexcan_fd_frame_t canfd_frame;
		flexcan_fd_frame_t canfd_rxFrame[RX_QUEUE_BUFFER_SIZE * 2];

		flexcan_frame_t frame;
		flexcan_frame_t rxFrame[RX_QUEUE_BUFFER_SIZE * 2];
	} u;

	uint32_t txIdentifier;
	uint32_t rxIdentifier;

	uint8_t node_type;
	uint8_t test_type;
	bool use_canfd;

	bool sys_exit;

	volatile status_t rxStatus;
	volatile uint32_t rxQueueNum;
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief User read message buffer function
 */
static status_t User_ReadRxMb(CAN_Type *base, uint8_t mbIdx, flexcan_frame_t *pRxFrame)
{
    /* Assertion. */
    assert(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    assert(NULL != pRxFrame);

    status_t status;
    uint32_t cs_temp;
    uint8_t rx_code;
    uint32_t can_id = 0;
    /* Read CS field of Rx Message Buffer to lock Message Buffer. */
    cs_temp = base->MB[mbIdx].CS;
    can_id  = base->MB[mbIdx].ID;

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

        /* Store Message Payload. */
        pRxFrame->dataWord0 = base->MB[mbIdx].WORD0;
        pRxFrame->dataWord1 = base->MB[mbIdx].WORD1;

        /* Restore original Rx value*/
        base->MB[mbIdx].ID = FLEXCAN_ID_STD(RX_MB_ID_AFTER_MASK);

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

static status_t User_FDReadRxMb(CAN_Type *base, uint8_t mbIdx, flexcan_fd_frame_t *pRxFrame)
{
    /* Assertion. */
    assert(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    assert(NULL != pRxFrame);

    status_t status;
    uint32_t cs_temp;
    uint8_t rx_code;
    uint32_t can_id = 0;
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

        /* Restore original Rx value*/
        base->MB[mbIdx].ID = FLEXCAN_ID_STD(RX_MB_ID_AFTER_MASK);

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
                    if (ctx->use_canfd)
                        (void)User_FDReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &ctx->u.canfd_rxFrame[i]);
                    else
                        (void)User_ReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &ctx->u.rxFrame[i]);

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
                    if (ctx->use_canfd)
                        status = User_FDReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &ctx->u.canfd_rxFrame[i]);
                    else
                        status = User_ReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &ctx->u.rxFrame[i]);

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
        if (ctx->use_canfd)
            (void)FLEXCAN_ReadFDRxMb(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &ctx->u.canfd_frame);
        else
            (void)FLEXCAN_ReadRxMb(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &ctx->u.frame);

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

    log_debug("enter\n");

    /* Select mailbox ID. */
    if (node_type == CAN_NODE_A)
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
    if (ctx->use_canfd && FLEXCAN_FDCalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.baudRate, flexcanConfig.baudRateFD,
                                                EXAMPLE_CAN_CLK_FREQ, &timing_config)) {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    } else if (!ctx->use_canfd && FLEXCAN_CalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.baudRate, EXAMPLE_CAN_CLK_FREQ,
                                                &timing_config)) {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    } else {
        log_info("No found Improved Timing Configuration. Just used default configuration\n\n");
    }
#endif

    log_info("init CAN with clock freq %lu Hz\n", EXAMPLE_CAN_CLK_FREQ);

    if (ctx->use_canfd) {
        FLEXCAN_FDInit(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ, BYTES_IN_MB, true);
        CAN_Type *base = EXAMPLE_CAN;
        if (!(base->MCR & CAN_MCR_FDEN_MASK)) {
            ctx->sys_exit = true;
            log_err("CAN FD not supported\n");
            return;
        }
    } else {
        FLEXCAN_Init(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ);
    }

    if (test_type == 1)
    {
        /* Setup Rx Message Buffer. */
        mbConfig.format = kFLEXCAN_FrameFormatStandard;
        mbConfig.type   = kFLEXCAN_FrameTypeData;
        mbConfig.id     = FLEXCAN_ID_STD(ctx->txIdentifier);

        if (ctx->use_canfd) {
            FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
            FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
        } else {
            FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
            FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
        }

        /* Enable Rx Message Buffer interrupt. */
        os_irq_register(EXAMPLE_FLEXCAN_IRQn, EXAMPLE_FLEXCAN_IRQHandler, ctx, OS_IRQ_PRIO_DEFAULT + 1);

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

        os_irq_register(EXAMPLE_FLEXCAN_IRQn, test_flexcan_irq_handler, ctx, OS_IRQ_PRIO_DEFAULT + 1);
        os_irq_enable(EXAMPLE_FLEXCAN_IRQn);

        /* Set Rx Masking mechanism. */
        FLEXCAN_SetRxMbGlobalMask(EXAMPLE_CAN, FLEXCAN_RX_MB_STD_MASK(ctx->rxIdentifier, 0, 0));

        /* Setup Rx Message Buffer. */
        mbConfig.format = kFLEXCAN_FrameFormatStandard;
        mbConfig.type   = kFLEXCAN_FrameTypeData;
        mbConfig.id     = FLEXCAN_ID_STD(ctx->rxIdentifier);

        if (ctx->use_canfd)
            FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
        else
            FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);

        /* Setup Tx Message Buffer. */
        if (ctx->use_canfd)
            FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
        else
            FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);

        if (node_type == CAN_NODE_A)
        {
            log_info("Press any key to trigger one-shot transmission\n\n");
            ctx->u.frame.dataByte0 = 0;
        }
        else
        {
            log_info("Start to Wait data from Node A\n\n");
        }
    }
    else if (test_type == 3)
    {
        if (node_type == CAN_NODE_A)
        {
            /* Setup Tx Message Buffer. */
            if (ctx->use_canfd)
                FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
            else
                FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);

            ctx->u.frame.dataByte0 = 0;
            ctx->u.frame.dataByte1 = 0x55;
        }
        else
        {
            FLEXCAN_EnableInterrupts(EXAMPLE_CAN, (uint32_t)kFLEXCAN_BusOffInterruptEnable |
                                                  (uint32_t)kFLEXCAN_ErrorInterruptEnable |
                                                  (uint32_t)kFLEXCAN_RxWarningInterruptEnable);
            /* Register interrupt. */
            os_irq_register(EXAMPLE_FLEXCAN_IRQn, test_flexcan_irq_handler, ctx, OS_IRQ_PRIO_DEFAULT + 1);
            /* Setup Rx Message Buffer. */
            mbConfig.format = kFLEXCAN_FrameFormatStandard;
            mbConfig.type   = kFLEXCAN_FrameTypeData;
            mbConfig.id     = FLEXCAN_ID_STD(RX_MB_ID_AFTER_MASK);

            for (i = 0U; i < RX_QUEUE_BUFFER_SIZE * 2U; i++)
            {
                /* Setup Rx individual ID mask. */
                FLEXCAN_SetRxIndividualMask(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i,
                                        FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK, 0, 0));
                if (ctx->use_canfd)
                    FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &mbConfig, true);
                else
                    FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &mbConfig, true);
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
    log_debug("end\n");
}

static void test_flexcan_transmit(struct can_ctx *ctx)
{
    uint8_t test_type = ctx->test_type;
    uint8_t node_type = ctx->node_type;
    static uint32_t TxCount = 1;
    uint32_t i;

    log_debug("enter\n");

    if (ctx->sys_exit)
        goto exit;

    if ((node_type == CAN_NODE_A) || (test_type == 1))
    {
        ctx->u.frame.id     = FLEXCAN_ID_STD(ctx->txIdentifier);
        ctx->u.frame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
        ctx->u.frame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
        ctx->u.frame.length = (uint8_t)DLC;
        if (ctx->use_canfd)
            ctx->u.canfd_frame.brs = (uint8_t)1U;

        if (test_type == 1)
        {
            if (ctx->use_canfd) {
                for (i = 0; i < DWORD_IN_MB; i++)
                {
                    ctx->u.canfd_frame.dataWord[i] = i;
                }
            } else {
                ctx->u.frame.dataWord0 = CAN_WORD0_DATA_BYTE_0(0x11) | CAN_WORD0_DATA_BYTE_1(0x22) | CAN_WORD0_DATA_BYTE_2(0x33) |
                                CAN_WORD0_DATA_BYTE_3(0x44);
                ctx->u.frame.dataWord1 = CAN_WORD1_DATA_BYTE_4(0x55) | CAN_WORD1_DATA_BYTE_5(0x66) | CAN_WORD1_DATA_BYTE_6(0x77) |
                                CAN_WORD1_DATA_BYTE_7(0x88);
            }

            log_info("Send message from MB%d to MB%d\n", TX_MESSAGE_BUFFER_NUM, RX_MESSAGE_BUFFER_NUM);
            if (ctx->use_canfd) {
                for (i = 0; i < DWORD_IN_MB; i++)
                {
                    log_info("tx word%d = 0x%x\n", i, ctx->u.canfd_frame.dataWord[i]);
                }
            } else {
                log_info("tx word0 = 0x%x\n", ctx->u.frame.dataWord0);
                log_info("tx word1 = 0x%x\n", ctx->u.frame.dataWord1);
            }

            /* Send data through Tx Message Buffer using polling function. */
            if (ctx->use_canfd)
                (void)FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->u.canfd_frame);
            else
                (void)FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->u.frame);

            /* Waiting for Message receive finish. */
            while (!ctx->rxComplete)
            {
                if (ctx->sys_exit)
                    goto exit;
            }

            log_info("Received message from MB%d\n", RX_MESSAGE_BUFFER_NUM);
            if (ctx->use_canfd) {
                for (i = 0; i < DWORD_IN_MB; i++)
                {
                    log_info("rx word%d = 0x%x\n", i, ctx->u.canfd_frame.dataWord[i]);
                }
            }
            log_info("rx word0 = 0x%x\n", ctx->u.frame.dataWord0);
            log_info("rx word1 = 0x%x\n", ctx->u.frame.dataWord1);

            /* Stop FlexCAN Send & Receive. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
            FLEXCAN_DisableMbInterrupts(EXAMPLE_CAN, (uint64_t)1U << RX_MESSAGE_BUFFER_NUM);
#else
            FLEXCAN_DisableMbInterrupts(EXAMPLE_CAN, (uint32_t)1U << RX_MESSAGE_BUFFER_NUM);
#endif

            log_info("==FlexCAN loopback functional example -- Finish.==\n");

        } else if (test_type == 2) {
            ctx->txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
            if (ctx->use_canfd) {
                ctx->txXfer.framefd = &ctx->u.canfd_frame;
                (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
            } else {
                ctx->txXfer.frame = &ctx->u.frame;
                (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
            }

            log_info("Transmission began\n");
            while (!ctx->txComplete)
            {
                if (ctx->sys_exit)
                    goto exit;
            };
            ctx->txComplete = false;

            log_info("Transmission sent\n");
            /* Start receive data through Rx Message Buffer. */
            ctx->rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
            if (ctx->use_canfd) {
                ctx->rxXfer.framefd = &ctx->u.canfd_frame;
                (void)FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
            } else {
                ctx->rxXfer.frame = &ctx->u.frame;
                (void)FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
            }

            /* Wait until Rx MB full. */
            while (!ctx->rxComplete)
            {
                if (ctx->sys_exit)
                    goto exit;
            };
            ctx->rxComplete = false;

            log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->u.frame.id >> CAN_ID_STD_SHIFT,
                     ctx->u.frame.dataByte0, ctx->u.frame.timestamp);
            ctx->u.frame.dataByte0++;
            ctx->u.frame.dataByte1 = 0x55;

            log_info("==FlexCAN interrupt functional example -- Finish.==\n");
        } else if (test_type == 3) {
            uint32_t times = 40;

            for (i = 0; i < times; i++)
            {
                if (ctx->use_canfd)
                    (void)FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->u.canfd_frame);
                else
                    (void)FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &ctx->u.frame);

                /* Wait for 200ms after every 2 RX_QUEUE_BUFFER_SIZE transmissions. */
                if ((TxCount % (RX_QUEUE_BUFFER_SIZE * 2U)) == 0U)
                    os_msleep(200);

                ctx->u.frame.dataByte0++;
                TxCount++;
            }
            log_info("Transmission done.\n\n");
            log_info("==FlexCAN PingPong functional example -- Finish.==\n");
        }
    } else {
        if (test_type == 2) {
            /* Before this , should first make node B enter STOP mode after FlexCAN
             * initialized with enableSelfWakeup=true and Rx MB configured, then A
             * sends ctx->frame N which wakes up node B. A will continue to send ctx->frame N
             * since no acknowledgement, then B received the second ctx->frame N(In the
             * application it seems that B received the ctx->frame that woke it up which
             * is not expected as stated in the reference manual, but actually the
             * output in the terminal B received is the same second ctx->frame N). */
            if (ctx->wakenUp) {
                log_info("B has been waken up!\n\n");
            }

            /* Start receive data through Rx Message Buffer. */
            ctx->rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
            if (ctx->use_canfd) {
                ctx->rxXfer.framefd = &ctx->u.canfd_frame;
                (void)FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
            } else {
                ctx->rxXfer.frame = &ctx->u.frame;
                (void)FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->rxXfer);
            }

            /* Wait until Rx receive full. */
            while (!ctx->rxComplete)
            {
                if (ctx->sys_exit)
                    goto exit;
            };
            ctx->rxComplete = false;

            log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->u.frame.id >> CAN_ID_STD_SHIFT,
                 ctx->u.frame.dataByte0, ctx->u.frame.timestamp);

            ctx->u.frame.id     = FLEXCAN_ID_STD(ctx->txIdentifier);
            ctx->txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
            if (ctx->use_canfd) {
                ctx->u.canfd_frame.brs = 1;
                ctx->txXfer.framefd = &ctx->u.canfd_frame;
                (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
            } else {
                ctx->txXfer.frame = &ctx->u.frame;
                (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &ctx->flexcanHandle, &ctx->txXfer);
            }

            while (!ctx->txComplete)
            {
                if (ctx->sys_exit)
                    goto exit;
            };
            ctx->txComplete = false;

            log_info("==FlexCAN interrupt functional example -- Finish.==\n");
        } else if (test_type == 3) {
            while (true)
            {
                /* Wait until Rx queue 1 full. */
                while (ctx->rxQueueNum != 1U)
                {
                    if (ctx->sys_exit)
                        goto exit;
                };
                ctx->rxQueueNum = 0;
                log_info("Read Rx MB from Queue 1.\n");
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                {
                    log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->u.rxFrame[i].id >> CAN_ID_STD_SHIFT,
                             ctx->u.rxFrame[i].dataByte0, ctx->u.rxFrame[i].timestamp);
                }
                /* Wait until Rx queue 2 full. */
                while (ctx->rxQueueNum != 2U)
                {
                    if (ctx->sys_exit)
                        goto exit;
                };
                ctx->rxQueueNum = 0;
                log_info("Read Rx MB from Queue 2.\n");
                for (; i < (RX_QUEUE_BUFFER_SIZE * 2U); i++)
                {
                    log_info("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\n", ctx->u.rxFrame[i].id >> CAN_ID_STD_SHIFT,
                             ctx->u.rxFrame[i].dataByte0, ctx->u.rxFrame[i].timestamp);
                }
                if (ctx->rxStatus == kStatus_FLEXCAN_RxOverflow) {
                    ctx->rxStatus = 0;
                    log_info("The data in the last MB %d in the queue 2 is overwritten\n", RX_QUEUE_BUFFER_END_2);
                }
                log_info("Wait Node A to trigger the next 8 messages!\n\n");
            }
            log_info("==FlexCAN PingPong functional example -- Finish.==\n");
        }
    }

exit:
    os_irq_unregister(EXAMPLE_FLEXCAN_IRQn);

    log_info("end\n");
}

void can_stats(void *priv)
{
	struct can_ctx *ctx = priv;

	(void)ctx;
	log_debug("not implemented\n");
}

int can_run(void *priv, struct event *e)
{
	struct can_ctx *ctx = priv;
	int err = -1;

	log_info("FLEXCAN running: type %d node %c\n",
			ctx->test_type, ctx->node_type ? 'B' : 'A');
	switch (e->type)
	{
		case EVENT_TYPE_START:
			test_flexcan_transmit(ctx);
			err = 0;
			break;
		default:
			log_err("Invalid event: %d\n", e->type);
			break;
	}

	return err;
}

void can_pre_exit(void *priv)
{
	struct can_ctx *ctx = priv;

	ctx->sys_exit = true;
}

static void *can_init(void *parameters, uint32_t test_type)
{
	struct industrial_config *cfg = parameters;
	uint32_t node_type = cfg->role;
	struct can_ctx *ctx = NULL;

	/* sanity check */
	if (node_type != CAN_NODE_A && node_type != CAN_NODE_B) {
		log_err("Invalid role (node type): %d\n", node_type);

		goto exit;
	}

	ctx = os_malloc(sizeof(struct can_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");

		goto exit;
	}

	memset(ctx, 0, sizeof(struct can_ctx));

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;
	ctx->use_canfd = cfg->protocol;
	ctx->node_type = node_type;
	ctx->test_type = test_type;
	ctx->sys_exit = false;

	log_debug("node %c\n", ctx->node_type ? 'B' : 'A');
	log_debug("test type %d\n", test_type);
	log_debug("used protocol %s\n", ctx->use_canfd ? "CAN-FD" : "CAN");

	hardware_flexcan_init();

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

/*=======================================================================================================================================*/
#define MAX_NODES					2
#define MAX_MESSAGE_BUFFERS			4
#define PROCESS_ALARM_PERIOD_US		1200U
#define MEGA						1000000U

extern void *os_counter;

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
	uint32_t message_buffer_n;
	struct mb_config mb_conf[MAX_MESSAGE_BUFFERS];
};

struct flexcan_ctx {
	void (*event_send)(void *, uint8_t, uint8_t);
	void (*flexcanHandle)(void *);
	void *event_data;

	CAN_Type *base;
	uint32_t clk_freq;
	uint32_t bps;
	uint32_t global_irq_mask;
	uint64_t global_irq_count;

	/* Activated the given number message buffers */
	struct message_buffer mb[MAX_MESSAGE_BUFFERS];
	uint8_t mb_number;

	uint8_t node_type;
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

static void flexcan_irq_handler(void *data)
{
	struct flexcan_ctx *ctx = data;

	FLEXCAN_DisableMbInterrupts(ctx->base, ctx->global_irq_mask);

	if (ctx->event_send)
		ctx->event_send(ctx->event_data, EVENT_TYPE_IRQ, 0);
}

static void alarm_handler(const void *dev, uint8_t chan_id,
			  uint32_t irq_counter,
			  void *user_data)
{
	struct flexcan_ctx *ctx = user_data;

	if (ctx->event_send)
		ctx->event_send(ctx->event_data, EVENT_TYPE_TIMER, 0);

}

static void alarm_trigger(struct flexcan_ctx *ctx, uint32_t us)
{
	int err = 0;
	uint64_t ticks = os_counter_us_to_ticks(os_counter, us);

	ctx->alarm_cfg.ticks = ticks;
	ctx->alarm_cfg.flags = 0;
	ctx->alarm_cfg.user_data = ctx;
	ctx->alarm_cfg.callback = alarm_handler;

	err = os_counter_set_channel_alarm(os_counter, 0, &ctx->alarm_cfg);

	/* Counter set alarm failed */
	if (err == -1)
		ctx->alarm_err++;
}

uint32_t get_global_irq_mask(struct node_config node, uint8_t message_buffer_number)
{
	uint32_t mask = 0;

	for (int i = 0; i < message_buffer_number; i++)
		mask |= (uint32_t)1U << node.mb_conf[i].index;

	return mask;
}

static void flexcan_enable_interrupt(struct flexcan_ctx *ctx)
{
	os_irq_register(EXAMPLE_FLEXCAN_IRQn, ctx->flexcanHandle, ctx, OS_IRQ_PRIO_DEFAULT + 1);
	os_irq_enable(EXAMPLE_FLEXCAN_IRQn);
	FLEXCAN_EnableMbInterrupts(ctx->base, ctx->global_irq_mask);
}

static void flexcan_disable_interrupt(struct flexcan_ctx *ctx)
{
	FLEXCAN_DisableMbInterrupts(ctx->base, ctx->global_irq_mask);
	os_irq_disable(EXAMPLE_FLEXCAN_IRQn);
	os_irq_unregister(EXAMPLE_FLEXCAN_IRQn);
}

static void flexcan_config(struct flexcan_ctx *ctx)
{
	flexcan_config_t flexcanConfig;

	FLEXCAN_GetDefaultConfig(&flexcanConfig);
	flexcanConfig.enableIndividMask = true;
	if (!ctx) {
		log_err("No context found\n");
		return;
	}

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
	if (ctx->use_canfd) {
		FLEXCAN_FDInit(ctx->base, &flexcanConfig, ctx->clk_freq , BYTES_IN_MB, true);
		if (!(ctx->base->MCR & CAN_MCR_FDEN_MASK))
			log_err("CAN FD not supported\n");
	} else {
		FLEXCAN_Init(ctx->base, &flexcanConfig, ctx->clk_freq);
	}
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
	uint32_t node_type = cfg->role;
	struct flexcan_ctx *ctx = NULL;

	/* Sanity check */
	if (node_type != CAN_NODE_A && node_type != CAN_NODE_B) {
		log_err("Invalid role (node type): %u\n", node_type);
		goto exit;
	}

	ctx = os_malloc(sizeof(struct flexcan_ctx));
	if (!ctx) {
		log_err("Memory allocation error\n");
		goto exit;
	}

	memset(ctx, 0, sizeof(struct flexcan_ctx));

	ctx->base = EXAMPLE_CAN;
	ctx->clk_freq = EXAMPLE_CAN_CLK_FREQ;

	ctx->event_send = cfg->event_send;
	ctx->event_data = cfg->event_data;
	ctx->use_canfd = cfg->protocol;

	/* User input values */
	ctx->node_type = node_type;
	ctx->test_type = test_type;
	ctx->mb_number = 4;

	if ((ctx->mb_number > MAX_MESSAGE_BUFFERS) || (ctx->mb_number > node_configs[ctx->node_type].message_buffer_n)) {
		log_err("The given message buffer number is unsupported\n");
		goto exit;
	}

	/* Process values */
	ctx->flexcanHandle = flexcan_irq_handler;
	ctx->global_irq_mask = get_global_irq_mask(node_configs[ctx->node_type], ctx->mb_number);
	ctx->global_irq_count = 0;

	/* Load message buffers config */
	for (int i = 0; i < ctx->mb_number; i++)
		ctx->mb[i].conf = node_configs[ctx->node_type].mb_conf[i];

exit:
	return ctx;
}

void *flexcan_init(void *parameters)
{
	struct flexcan_ctx *ctx;

	log_info("START\n");
	ctx = get_ctx(parameters, 3);
	if (!ctx) {
		log_err("Context not found\n");
		return NULL;
	}

	hardware_flexcan_init();
	flexcan_config(ctx);

	return ctx;
}

int flexcan_run(void *priv, struct event *e)
{
	struct flexcan_ctx *ctx = priv;
	uint8_t status = -1;
	uint32_t flag = 0;

	if (!ctx) {
		log_err("Context not found\n");
		return status;
	}

	switch (e->type) {
	case EVENT_TYPE_START:
		for (int i= 0; i < ctx->mb_number; i++) {
			if (node_configs[ctx->node_type].mb_conf[i].tx) {
				mb_tx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
			} else {
				mb_rx_process(&ctx->mb[i], ctx->base, ctx->use_canfd, e);
			}
		}
		flexcan_enable_interrupt(ctx);
		os_counter_start(os_counter);
		alarm_trigger(ctx, PROCESS_ALARM_PERIOD_US);

		status = 0;
		break;
	case EVENT_TYPE_IRQ:
		ctx->global_irq_count++;
		flag = FLEXCAN_GetMbStatusFlags(ctx->base, ctx->global_irq_mask);
		FLEXCAN_ClearMbStatusFlags(ctx->base, flag);

		for (int i = 0; i < ctx->mb_number; i++) {
			/* Mailbox (index = N) is defined in mb[N-1] */
			if (flag & (1 << (i + 1))) {
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
		alarm_trigger(ctx, PROCESS_ALARM_PERIOD_US);

		status = 0;
		break;
	default:
		log_err("Invalid event: %u\n", e->type);
		break;
	}

	return status;
}

void flexcan_stats(void *priv)
{
	struct flexcan_ctx *ctx = priv;

	log_info("|Mbit/s: %9u|TX period µs: %5u|global irq: %+u|\n", ctx->bps/MEGA, PROCESS_ALARM_PERIOD_US, ctx->global_irq_count);
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

void flexcan_exit(void *priv)
{
	struct flexcan_ctx *ctx = priv;

	os_counter_stop(os_counter);
	flexcan_disable_interrupt(ctx);
	os_free(ctx);

	log_info("END\n");
}