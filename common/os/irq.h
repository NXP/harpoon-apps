/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _COMMON_IRQ_H_
#define _COMMON_IRQ_H_

#if defined(OS_ZEPHYR)
  #include "zephyr/os/irq.h"
#elif defined(FSL_RTOS_FREE_RTOS)
  #include "freertos/os/irq.h"
#endif

#endif /* #ifndef _COMMON_IRQ_H_ */
