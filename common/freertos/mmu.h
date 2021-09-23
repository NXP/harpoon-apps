/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MMU_H_
#define _MMU_H_

#include "fsl_device_registers.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

static inline void MMU_init(void)
{
    return ARM_MMU_Initialize(true);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _MMU_H_ */
