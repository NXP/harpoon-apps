/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "board.h"
#include "net/port_enet.h"
#include "net/net_switch.h"
#include "virtio_board.h"
#include "virtio-net.h"
#include "app_virtio_config.h"
#include "os/stdio.h"
#include "os/unistd.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Unmask this macro to print switch statistics with 10s period. */
#define KEEP_SILENT

/* Task priorities. */
#define virtio_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void virtio_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
	BaseType_t xResult;

	virtio_board_init();

	xResult = xTaskCreate(virtio_task, "Virtio_task", configMINIMAL_STACK_SIZE + 100, NULL, virtio_task_PRIORITY, NULL);
	assert(xResult == pdPASS);

	vTaskStartScheduler();
	for (;;)
		;

	return xResult;
}

static void virtio_task(void *pvParameters)
{
	int ret;
	void *switch_dev;

	os_printf("\r\nStarting Virtio networking backend...\r\n");

	switch_dev = switch_init();
	if (!switch_dev) {
		os_printf("Networking switch initialization failed!\r\n");
		goto err;
	}

	ret = virtio_net_init((void *)VIRTIO_NET_MEM_BASE, switch_dev);
	os_printf("virtio network device initialization %s!\r\n", ret ? "failed": "succeed");

	ret = enet_port_init(switch_dev);
	os_printf("Switch enabled with enet remote port %s!\r\n", ret ? "failed": "succeed");

#ifndef KEEP_SILENT
	/* dead loop */
	switch_print_stats(switch_dev);
#endif
err:
	vTaskSuspend(NULL);
}
