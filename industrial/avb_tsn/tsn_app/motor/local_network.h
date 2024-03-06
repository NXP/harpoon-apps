/*
 * Copyright 2019, 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LOCAL_NETWORK_H_
#define _LOCAL_NETWORK_H_

#include "controller.h"
#include "io_device.h"

#if BUILD_MOTOR_IO_DEVICE == 1
void local_bind_controller_io_device(struct controller_ctx *controller, struct io_device_ctx *io_device);
void local_controller_transmit(struct controller_ctx *ctx, struct msg_set_iq *msg_to_send);
void local_io_device_transmit(struct io_device_ctx *ctx, struct msg_feedback *msg_send);
#else
static inline int local_bind_controller_io_device(struct controller_ctx *controller, struct io_device_ctx *io_device)
{
    return 0;
}

static inline int local_controller_transmit(struct controller_ctx *ctx, struct msg_set_iq *msg_to_send)
{
    return 0;
}

static inline int local_io_device_transmit(struct io_device_ctx *ctx, struct msg_feedback *msg_to_send)
{
    return 0;
}
#endif

#endif /* _LOCAL_NETWORK_H_ */
