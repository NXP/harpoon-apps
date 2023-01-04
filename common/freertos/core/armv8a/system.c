/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "gic_v3.h"

void SystemInitHook (void) {
	GIC_Enable(1);
}
