/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RPMSG_H_
#define _RPMSG_H_

int rpmsg_init(uint32_t dst);
void rpmsg_deinit(int fd);
int rpmsg_send(void *fd_p, const void *data, uint32_t len);
int rpmsg_recv(void *fd_p, void *data, uint32_t len);

#endif /* _RPMSG_H_ */
