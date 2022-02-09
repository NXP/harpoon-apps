/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FREERTOS_COUNTER_H_
#define _FREERTOS_COUNTER_H_

/* OSAL includes.*/
#include "os/counter.h"

/* Freescale includes. */
#include "fsl_common.h"
#include "fsl_gpt.h"

/* TODO: Use this to avoid exporting global variables */
struct os_counter_gpt_dev {
    GPT_Type *base;
    IRQn_Type *irqn;
    void *cb;
};

#define OS_COUNTER_ALARM_CFG_ABSOLUTE (1 << 0)

#endif /* #ifndef _FREERTOS_COUNTER_H_ */
