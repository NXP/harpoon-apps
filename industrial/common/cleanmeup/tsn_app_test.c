/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "init.h"
#include "os/init.h"
#include "os/stdlib.h"
#include "system_config.h"
#include "genavb.h"
#include "irq.h"
#include "os/stdio.h"
#include "enet_qos_drv.h"

static struct genavb_handle *s_genavb_handle = NULL;

extern void BOARD_GPT_0_IRQ_HANDLER(void);
extern void BOARD_GPT_1_IRQ_HANDLER(void);

void test_tsn_endpoint(void)
{
    struct genavb_config *genavb_config;

    os_printf("%s: Start\r\n", __func__);

    enet_qos_hardware_init();

    genavb_config = os_malloc(sizeof(struct genavb_config));

    if (!genavb_config) {
        os_printf("%s: Unable to allocat genavb_config\r\n", __func__);
        goto exit;
    }

    genavb_get_default_config(genavb_config);
    genavb_set_config(genavb_config);

    irq_register(BOARD_GPT_0_IRQ, (void (*)(void(*)))BOARD_GPT_0_IRQ_HANDLER, NULL);
    irq_register(BOARD_GPT_1_IRQ, (void (*)(void(*)))BOARD_GPT_1_IRQ_HANDLER, NULL);

    if (genavb_init(&s_genavb_handle, 0) != GENAVB_SUCCESS) {
        os_printf("%s: Unable to init genavb stack\r\n", __func__);
        s_genavb_handle = NULL;
        goto exit;
    }

exit:
    if (genavb_config)
        os_free(genavb_config);
    os_printf("%s: Done\r\n", __func__);
}

