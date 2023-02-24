/*
 * Copyright 2023 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "os/assert.h"

#include "hlog.h"

#define STACK_SIZE       4096
#define MAX_TC_THREADS      2

K_THREAD_STACK_DEFINE(hello_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(tictac_stack, STACK_SIZE);

static struct main_ctx {
	struct k_thread tc_thread[MAX_TC_THREADS];
	int threads_running_count;
} main_ctx;

static void hello_func(void *p1, void *p2, void *p3)
{
	struct main_ctx *ctx = p1;

	log_raw(INFO, "\r\n");
	log_info("Hello world.\n");

	do {
		k_msleep(500);

		log_info("%d threads running\n", ctx->threads_running_count);

		k_thread_suspend(k_current_get());

	} while(1);
}

static void tictac_func(void *p1, void *p2, void *p3)
{
	unsigned long long count = 0;

	do {
		k_msleep(1000);

		if (++count % 2)
			log_raw(INFO, "tic ");
		else
			log_raw(INFO, "tac ");

		if (!(count % 20))
			log_raw(INFO, "\r\n");

	} while(1);
}

int main(void)
{
	struct main_ctx *ctx = &main_ctx;
	struct k_thread *hello_thread;
	struct k_thread *tictac_thread;

	/* tic tac thread */
	tictac_thread = &ctx->tc_thread[ctx->threads_running_count++];
	k_thread_create(tictac_thread, tictac_stack, STACK_SIZE,
			tictac_func, NULL, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_FOREVER);

	/* Hello thread */
	hello_thread = &ctx->tc_thread[ctx->threads_running_count++];
	k_thread_create(hello_thread, hello_stack, STACK_SIZE,
		hello_func, ctx, NULL, NULL,
		K_HIGHEST_THREAD_PRIO, 0, K_FOREVER);

	/* Start threads */
	k_thread_start(tictac_thread);
	k_thread_start(hello_thread);

	do {
		k_msleep(100);

	} while(1);

	return 0;
}
