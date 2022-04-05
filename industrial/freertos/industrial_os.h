/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_FREERTOS_H_
#define _INDUSTRIAL_FREERTOS_H_

struct thread_cfg {
	char name[32];
	unsigned int nb_threads;
	unsigned int priority;
};

#endif /* _INDUSTRIAL_FREERTOS_H_ */
