/*
 * Copyright 2021 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RT_TESTCASE_H_
#define _RT_TESTCASE_H_

/* Test Case Setup */

/* Whether add extra IRQ load thread */
//#define WITH_IRQ_LOAD

/* Whether add extra CPU load thread */
#define WITH_CPU_LOAD
#ifdef WITH_CPU_LOAD
/* Add Semaphore load in CPU load thread */
//#define WITH_CPU_LOAD_SEM
/* 10Hz frequency to invalidate cache */
#define WITH_INVD_CACHE
#endif

/* No any print message during testing */
//#define SILENT_TESTING

/* Enabled to record latency histogram */
#define ENABLE_HISTOGRAM

/* Number of Testing Loop */
#define TESTING_LOOP_NUM	60000

/* Time for Counter Alarm timeout (us) */
#define COUNTER_PERIOD_US_VAL	20000

/* Thread CPU Binding for SMP kernel */
#ifdef CONFIG_SMP
/* Whether bind threads to specified CPU Core */
#define THREAD_CPU_BINDING

#ifdef THREAD_CPU_BINDING
/* GPT Testing Thread CPU Binding */
#define GPT_CPU_BINDING		1
/* Print Thread CPU Binding */
#define PRINT_CPU_BINDING	0
/* CPU Load Core Binding */
#ifdef WITH_CPU_LOAD
#define LOAD_CPU_BINDING	1
#endif
#endif /* THREAD_CPU_BINDING */

#endif /* CONFIG_SMP */

#define IGNORE_FIRST_CYCLES	10

#endif /* _RT_TESTCASE_H_ */
