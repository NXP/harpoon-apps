/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_flexcan.h"
#include "fsl_gpio.h"

#include "flexcan_test.h"

#include "os/irq.h"

#include "rtos_abstraction_layer.h"

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
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
flexcan_handle_t flexcanHandle;
volatile bool txComplete = false;
volatile bool rxComplete = false;
volatile bool wakenUp    = false;
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
uint32_t test_type = 0;
uint8_t node_type = 'A';
volatile status_t rxStatus = 0;
volatile uint32_t rxQueueNum = 0;

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
static void User_TransferHandleIRQ(CAN_Type *base)
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
            rxStatus = kStatus_FLEXCAN_ErrorStatus;
            /* Clear FlexCAN Error and Status Interrupt. */
            FLEXCAN_ClearStatusFlags(base, (uint32_t)kFLEXCAN_TxWarningIntFlag | (uint32_t)kFLEXCAN_RxWarningIntFlag |
                                               (uint32_t)kFLEXCAN_BusOffIntFlag | (uint32_t)kFLEXCAN_ErrorIntFlag);
        }
        else if (0U != (EsrStatus & (uint32_t)kFLEXCAN_WakeUpIntFlag))
        {
            rxStatus = kStatus_FLEXCAN_WakeUp;
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
                rxQueueNum = 1U;
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                {
                    /* Default receive message ID is 0x321, and after individual mask 0xFF, can match the Rx queue MB ID
                       0x21. Change queue 1 MBs individual ID mask to 0x3FF, will make it can't match the Rx queue MB
                       ID, so queue 1 will ignore the next CAN/CANFD messages and the queue 2 will receive the messages.
                     */
                    FLEXCAN_SetRxIndividualMask(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i,
                                                FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK | TX_MB_ID, 0, 0));
                    (void)User_ReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &rxFrame[i]);
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
                rxQueueNum = 2U;
                /* Restore queue 1 ID mask to make it can receive the next CAN/CANFD messages again. */
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                    FLEXCAN_SetRxIndividualMask(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i,
                                                FLEXCAN_RX_MB_STD_MASK(RX_MB_ID_MASK, 0, 0));
                for (; i < (RX_QUEUE_BUFFER_SIZE * 2U); i++)
                {
                    status = User_ReadRxMb(EXAMPLE_CAN, RX_QUEUE_BUFFER_BASE + i, &rxFrame[i]);

                    /* Clear queue 2 Message Buffer receive status. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
                    FLEXCAN_ClearMbStatusFlags(base, (uint64_t)1U << (RX_QUEUE_BUFFER_BASE + i));
#else
                    FLEXCAN_ClearMbStatusFlags(base, (uint32_t)1U << (RX_QUEUE_BUFFER_BASE + i));
#endif
                    /* Due to enable the queue feature, the rx overflow may only occur in the last matched MB
                       (RX_QUEUE_BUFFER_END_2). */
                    if (((RX_QUEUE_BUFFER_BASE + i) == RX_QUEUE_BUFFER_END_2) && (status == kStatus_FLEXCAN_RxOverflow))
                        rxStatus = status;
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

    switch (status)
    {
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MESSAGE_BUFFER_NUM == result)
            {
                rxComplete = true;
            }
            break;

        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                txComplete = true;
            }
            break;

        case kStatus_FLEXCAN_WakeUp:
            wakenUp = true;
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
        (void)FLEXCAN_ReadFDRxMb(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &frame);
#else
        (void)FLEXCAN_ReadRxMb(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &frame);
#endif
        rxComplete = true;
    }
}



void test_flexcan_irq_handler(void *data)
{
    switch (test_type)
    {
        case 2:
            FLEXCAN_TransferHandleIRQ(EXAMPLE_CAN, &flexcanHandle);
            break;

        case 3:
            User_TransferHandleIRQ(EXAMPLE_CAN);
            break;

        default:
            break;
    }

}


void test_flexcan_setup(void)
{
    flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t mbConfig;
    uint32_t i;

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
        rtos_printf("No found Improved Timing Configuration. Just used default configuration\r\n\r\n");
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
        rtos_printf("No found Improved Timing Configuration. Just used default configuration\r\n\r\n");
    }
#endif
#endif

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
        mbConfig.id     = FLEXCAN_ID_STD(txIdentifier);
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

        GIC_SetInterfacePriorityMask(0xf0);
        /* Enable Rx Message Buffer interrupt. */
        os_irq_register(EXAMPLE_FLEXCAN_IRQn, EXAMPLE_FLEXCAN_IRQHandler, NULL, OS_IRQ_PRIO_DEFAULT + 1);

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
        FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);

        os_irq_register(EXAMPLE_FLEXCAN_IRQn, test_flexcan_irq_handler, NULL, OS_IRQ_PRIO_DEFAULT + 1);
        os_irq_enable(EXAMPLE_FLEXCAN_IRQn);

        /* Set Rx Masking mechanism. */
        FLEXCAN_SetRxMbGlobalMask(EXAMPLE_CAN, FLEXCAN_RX_MB_STD_MASK(rxIdentifier, 0, 0));

        /* Setup Rx Message Buffer. */
        mbConfig.format = kFLEXCAN_FrameFormatStandard;
        mbConfig.type   = kFLEXCAN_FrameTypeData;
        mbConfig.id     = FLEXCAN_ID_STD(rxIdentifier);
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
            rtos_printf("Press any key to trigger one-shot transmission\r\n\r\n");
            frame.dataByte0 = 0;
        }
        else
        {
            rtos_printf("Start to Wait data from Node A\r\n\r\n");
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
            frame.dataByte0 = 0;
            frame.dataByte1 = 0x55;
        }
        else
        {
            FLEXCAN_EnableInterrupts(EXAMPLE_CAN, (uint32_t)kFLEXCAN_BusOffInterruptEnable |
                                                  (uint32_t)kFLEXCAN_ErrorInterruptEnable |
                                                  (uint32_t)kFLEXCAN_RxWarningInterruptEnable);
            /* Register interrupt. */
            os_irq_register(EXAMPLE_FLEXCAN_IRQn, test_flexcan_irq_handler, NULL, OS_IRQ_PRIO_DEFAULT + 1);
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

            rtos_printf("Start to Wait data from Node A\r\n\r\n");
        }
    }
}

void test_flexcan(void)
{
    uint32_t i;
    static uint32_t TxCount = 1;

    /* GPIO5_IO05 is used to control CAN1_STBY which is enabled active high */
    gpio_pin_config_t config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO5, 5U, &config);

    while (test_type == 0)
    {

        rtos_printf("********* Select FlexCAN test mode (1-3) *********\r\n");
        rtos_printf("1 - Loopback\r\n");
        rtos_printf("2 - Interrupt transfer\r\n");
        rtos_printf("3 - PingPong\r\n");
        SCANF("%d", &test_type);
        if (test_type < 1 || test_type > 3)
        {
            test_type = 0;
        }
        rtos_printf("\r\n");
    }

    switch (test_type)
    {
        case 1:
            rtos_printf("********* FLEXCAN Loopback EXAMPLE *********\r\n");
            rtos_printf("    Message format: Standard (11 bit id)\r\n");
            rtos_printf("    Loopback Mode: Enabled\r\n");
            rtos_printf("*********************************************\r\n\r\n");
            break;

        case 2:
            rtos_printf("********* FLEXCAN Interrupt EXAMPLE *********\r\n");
            rtos_printf("    Message format: Standard (11 bit id)\r\n");
            rtos_printf("    Message buffer %d used for Rx.\r\n", RX_MESSAGE_BUFFER_NUM);
            rtos_printf("    Message buffer %d used for Tx.\r\n", TX_MESSAGE_BUFFER_NUM);
            rtos_printf("    Interrupt Mode: Enabled\r\n");
            rtos_printf("    Operation Mode: TX and RX --> Normal\r\n");
            rtos_printf("*********************************************\r\n\r\n");
            break;

        case 3:
            rtos_printf("********* FLEXCAN PingPong Buffer Example *********\r\n");
            rtos_printf("    Message format: Standard (11 bit id)\r\n");
            rtos_printf("    Node B Message buffer %d to %d used as Rx queue 1.\r\n", RX_QUEUE_BUFFER_BASE, RX_QUEUE_BUFFER_END_1);
            rtos_printf("    Node B Message buffer %d to %d used as Rx queue 2.\r\n", RX_QUEUE_BUFFER_END_1 + 1U, RX_QUEUE_BUFFER_END_2);
            rtos_printf("    Node A Message buffer %d used as Tx.\r\n", TX_MESSAGE_BUFFER_NUM);
            rtos_printf("*********************************************\r\n\r\n");
            break;

        default:
            break;
    }

    if(test_type == 2 || test_type == 3)
    {
        do
        {
            rtos_printf("Please select local node as A or B:\r\n");
            rtos_printf("Note: Node B should start first.\r\n");
            rtos_printf("Node:");
            node_type = GETCHAR();
            rtos_printf("%c", node_type);
            rtos_printf("\r\n");
        } while ((node_type != 'A') && (node_type != 'B') && (node_type != 'a') && (node_type != 'b'));
    }

    /* Select mailbox ID. */
    if ((node_type == 'A') || (node_type == 'a'))
    {
        txIdentifier = 0x321;
        rxIdentifier = 0x123;
    }
    else
    {
        txIdentifier = 0x123;
        rxIdentifier = 0x321;
    }

    test_flexcan_setup();

    if ((node_type == 'A') || (node_type == 'a') || test_type == 1)
    {
        frame.id     = FLEXCAN_ID_STD(txIdentifier);
        frame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
        frame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
        frame.length = (uint8_t)DLC;
#if (defined(USE_CANFD) && USE_CANFD)
        frame.brs = (uint8_t)1U;
#endif
        if (test_type == 1)
        {
#if (defined(USE_CANFD) && USE_CANFD)
            for (i = 0; i < DWORD_IN_MB; i++)
            {
               frame.dataWord[i] = i;
            }
#else
            frame.dataWord0 = CAN_WORD0_DATA_BYTE_0(0x11) | CAN_WORD0_DATA_BYTE_1(0x22) | CAN_WORD0_DATA_BYTE_2(0x33) |
                              CAN_WORD0_DATA_BYTE_3(0x44);
            frame.dataWord1 = CAN_WORD1_DATA_BYTE_4(0x55) | CAN_WORD1_DATA_BYTE_5(0x66) | CAN_WORD1_DATA_BYTE_6(0x77) |
                              CAN_WORD1_DATA_BYTE_7(0x88);
#endif

            rtos_printf("Send message from MB%d to MB%d\r\n", TX_MESSAGE_BUFFER_NUM, RX_MESSAGE_BUFFER_NUM);
#if (defined(USE_CANFD) && USE_CANFD)
            for (i = 0; i < DWORD_IN_MB; i++)
            {
                rtos_printf("tx word%d = 0x%x\r\n", i, frame.dataWord[i]);
            }
#else
            rtos_printf("tx word0 = 0x%x\r\n", frame.dataWord0);
            rtos_printf("tx word1 = 0x%x\r\n", frame.dataWord1);
#endif

            /* Send data through Tx Message Buffer using polling function. */
#if (defined(USE_CANFD) && USE_CANFD)
            (void)FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &frame);
#else
            (void)FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &frame);
#endif

            /* Waiting for Message receive finish. */
            while (!rxComplete)
            {
            }

            rtos_printf("\r\nReceived message from MB%d\r\n", RX_MESSAGE_BUFFER_NUM);
#if (defined(USE_CANFD) && USE_CANFD)
            for (i = 0; i < DWORD_IN_MB; i++)
            {
                rtos_printf("rx word%d = 0x%x\r\n", i, frame.dataWord[i]);
            }
#else
            rtos_printf("rx word0 = 0x%x\r\n", frame.dataWord0);
            rtos_printf("rx word1 = 0x%x\r\n", frame.dataWord1);
#endif

            /* Stop FlexCAN Send & Receive. */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
            FLEXCAN_DisableMbInterrupts(EXAMPLE_CAN, (uint64_t)1U << RX_MESSAGE_BUFFER_NUM);
#else
            FLEXCAN_DisableMbInterrupts(EXAMPLE_CAN, (uint32_t)1U << RX_MESSAGE_BUFFER_NUM);
#endif

            rtos_printf("\r\n==FlexCAN loopback functional example -- Finish.==\r\n");

        } else if (test_type == 2) {
            GETCHAR();

            txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            txXfer.framefd = &frame;
            (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
#else
            txXfer.frame = &frame;
            (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
#endif

            rtos_printf("Transmission began\r\n");
            while (!txComplete)
            {
            };
            txComplete = false;

            rtos_printf("Transmission sent\r\n");
            /* Start receive data through Rx Message Buffer. */
            rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            rxXfer.framefd = &frame;
            (void)FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
#else
            rxXfer.frame = &frame;
            (void)FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
#endif

            /* Wait until Rx MB full. */
            while (!rxComplete)
            {
            };
            rxComplete = false;

            rtos_printf("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\r\n", frame.id >> CAN_ID_STD_SHIFT,
                     frame.dataByte0, frame.timestamp);
            frame.dataByte0++;
            frame.dataByte1 = 0x55;

            rtos_printf("\r\n==FlexCAN interrupt functional example -- Finish.==\r\n");
        } else if (test_type == 3) {
            uint8_t index  = 0;
            uint32_t times = 0;
            rtos_printf("Please input the number of CAN/CANFD messages to be send and end with enter.\r\n");
            while (index != 0x0D)
            {
                index = GETCHAR();
                if ((index >= '0') && (index <= '9'))
                {
                    (void)PUTCHAR(index);
                    times = times * 10 + (index - 0x30U);
                }
            }
            rtos_printf("\r\n");

            for (i = 1; i <= times; i++)
            {
#if (defined(USE_CANFD) && USE_CANFD)
                (void)FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &frame);
#else
                (void)FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, &frame);
#endif
                /* Wait for 200ms after every 2 RX_QUEUE_BUFFER_SIZE transmissions. */
                if ((TxCount % (RX_QUEUE_BUFFER_SIZE * 2U)) == 0U)
                    SDK_DelayAtLeastUs(200000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

                frame.dataByte0++;
                TxCount++;
            }
            rtos_printf("Transmission done.\r\n\r\n");
            rtos_printf("\r\n==FlexCAN PingPong functional example -- Finish.==\r\n");
        }
    }
    else
    {
        if (test_type == 2)
        {
            /* Before this , should first make node B enter STOP mode after FlexCAN
             * initialized with enableSelfWakeup=true and Rx MB configured, then A
             * sends frame N which wakes up node B. A will continue to send frame N
             * since no acknowledgement, then B received the second frame N(In the
             * application it seems that B received the frame that woke it up which
             * is not expected as stated in the reference manual, but actually the
             * output in the terminal B received is the same second frame N). */
            if (wakenUp)
            {
                rtos_printf("B has been waken up!\r\n\r\n");
            }

            /* Start receive data through Rx Message Buffer. */
            rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            rxXfer.framefd = &frame;
            (void)FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
#else
            rxXfer.frame = &frame;
            (void)FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
#endif

            /* Wait until Rx receive full. */
            while (!rxComplete)
            {
            };
            rxComplete = false;

            rtos_printf("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\r\n", frame.id >> CAN_ID_STD_SHIFT,
                 frame.dataByte0, frame.timestamp);

            frame.id     = FLEXCAN_ID_STD(txIdentifier);
            txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
#if (defined(USE_CANFD) && USE_CANFD)
            frame.brs      = 1;
            txXfer.framefd = &frame;
            (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
#else
            txXfer.frame = &frame;
            (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
#endif

            while (!txComplete)
            {
            };
            txComplete = false;

            rtos_printf("\r\n==FlexCAN interrupt functional example -- Finish.==\r\n");
        } else if (test_type == 3) {
            while (true)
            {
                /* Wait until Rx queue 1 full. */
                while (rxQueueNum != 1U)
                {
                };
                rxQueueNum = 0;
                rtos_printf("Read Rx MB from Queue 1.\r\n");
                for (i = 0; i < RX_QUEUE_BUFFER_SIZE; i++)
                {
                    rtos_printf("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\r\n", rxFrame[i].id >> CAN_ID_STD_SHIFT,
                             rxFrame[i].dataByte0, rxFrame[i].timestamp);
                }
                /* Wait until Rx queue 2 full. */
                while (rxQueueNum != 2U)
                {
                };
                rxQueueNum = 0;
                rtos_printf("Read Rx MB from Queue 2.\r\n");
                for (; i < (RX_QUEUE_BUFFER_SIZE * 2U); i++)
                {
                    rtos_printf("Rx MB ID: 0x%3x, Rx MB data: 0x%x, Time stamp: %d\r\n", rxFrame[i].id >> CAN_ID_STD_SHIFT,
                             rxFrame[i].dataByte0, rxFrame[i].timestamp);
                }
                if (rxStatus == kStatus_FLEXCAN_RxOverflow)
                {
                    rxStatus = 0;
                    rtos_printf("The data in the last MB %d in the queue 2 is overwritten\r\n", RX_QUEUE_BUFFER_END_2);
                }
                rtos_printf("Wait Node A to trigger the next 8 messages!\r\n\r\n");
            }
            rtos_printf("\r\n==FlexCAN PingPong functional example -- Finish.==\r\n");
        }
    }
    os_irq_unregister(EXAMPLE_FLEXCAN_IRQn);
}
