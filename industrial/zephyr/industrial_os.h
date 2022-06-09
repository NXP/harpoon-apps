/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INDUSTRIAL_ZEPHYR_H_
#define _INDUSTRIAL_ZEPHYR_H_

struct thread_cfg {
	char name[32];
	unsigned int nb_threads; /* FXIME: unused yet */
	unsigned int affinity;   /* FXIME: unused yet */
	unsigned int priority;
};

#endif /* _INDUSTRIAL_ZEPHYR_H_ */
