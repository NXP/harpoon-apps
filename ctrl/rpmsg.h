/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RPMSG_H_
#define _RPMSG_H_

int rpmsg_init(uint32_t dst);
void rpmsg_deinit(int fd);
int rpmsg_send(int fd, const void *data, unsigned int len);
int rpmsg_recv(int fd, void *data, unsigned int *len);

#endif /* _RPMSG_H_ */
