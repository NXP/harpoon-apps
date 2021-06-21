/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MMU_H_
#define _MMU_H_
#include "arm_mmu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MMU_init(void) z_arm64_mmu_init(void)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void MMU_init(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _MMU_H_ */
