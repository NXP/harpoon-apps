/*
 * Copyright 2022, 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "rtos_abstraction_layer.h"
#include "monitoring_stats.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "log.h"

#define UDP_SERVER_IP   "192.168.1.1"
#define UDP_SERVER_PORT 7000

struct monitoring_stats_ctx {
    int socket_fd;
    struct sockaddr_in server_address;
};

int monitoring_stats_open(struct monitoring_stats_ctx **ctx)
{
    *ctx = rtos_malloc(sizeof(struct monitoring_stats_ctx));
    if (!*ctx)
        goto err;

    (*ctx)->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ((*ctx)->socket_fd < 0) {
        log_err("socket call failed\n");
        goto err_free;
    }

    memset(&(*ctx)->server_address, 0, sizeof(struct sockaddr_in));
    (*ctx)->server_address.sin_family = AF_INET;
    (*ctx)->server_address.sin_addr.s_addr = inet_addr(UDP_SERVER_IP);
    (*ctx)->server_address.sin_port = htons(UDP_SERVER_PORT);

    return 0;

err_free:
    rtos_free(*ctx);
err:
    return -1;
}

int monitoring_stats_send(struct monitoring_stats_ctx *ctx, struct monitoring_msg *datagram)
{
    int rc;

    rc = sendto(ctx->socket_fd, datagram, sizeof(struct monitoring_msg), MSG_DONTWAIT,
                (struct sockaddr *)&ctx->server_address, sizeof(ctx->server_address));

    if (rc < 0) {
        log_err("sendto() call failed, err: %d\n", rc);
        goto err;
    }
    return 0;

err:
    return -1;
}
