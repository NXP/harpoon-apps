/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rpmsg.h"

static ssize_t readn(int fd, void *buf, size_t len)
{
	size_t nr_left;
	ssize_t nr_read;
	char *pos;

	nr_left = len;
	pos = buf;
	while (nr_left > 0) {
		nr_read = read(fd, pos, nr_left);
		if (nr_read < 0)
			return nr_read;
		else if (!nr_read)
			break;
		nr_left -= nr_read;
		pos += nr_read;
	}

	return (len - nr_left);
}

static ssize_t writen(int fd, const void *buf, size_t len)
{
	size_t nr_left;
	ssize_t nr_write;
	char *pos;

	nr_left = len;
	pos = (char *)buf;
	while (nr_left > 0) {
		nr_write = write(fd, pos, nr_left);
		if (nr_write < 0)
			return nr_write;
		nr_left -= nr_write;
		pos += nr_write;
	}

	return (len - nr_left);
}

int rpmsg_send(void *fd_p, const void *data, uint32_t len)
{
	int ret;
	int fd = (uintptr_t)fd_p;

	ret = writen(fd, data, len);
	if (ret != len)
		return -1;

	return 0;
}

int rpmsg_recv(void *fd_p, void *data, uint32_t len)
{
	int ret;
	int fd = (uintptr_t)fd_p;

	ret = readn(fd, data, len);
	if (ret != len)
		return -1;

	return 0;
}

static int rpmsg_find_dev_idx(uint32_t dst)
{
	char rpmsg_dst[64];
	char buf[5];
	int fd, rc;
	int i;

	for (i = 0; ; i++) {
		if (snprintf(rpmsg_dst, sizeof(rpmsg_dst), "/sys/class/rpmsg/rpmsg%d/dst", i) < 0)
			break;

		fd = open(rpmsg_dst, O_RDONLY);
		if (fd < 0)
			break;

		rc = read(fd, buf, sizeof(buf) - 1);
		if (rc < 0) {
			close(fd);
			break;
		}

		buf[4] = '\0';

		if (dst == atoi(buf)) {
			close(fd);
			return i;
		}

		close(fd);
	}

	return -1;
}

int rpmsg_init(uint32_t dst)
{
	char rpmsg_dev[64];
	int idx, fd;

	idx = rpmsg_find_dev_idx(dst);
	if (idx < 0) {
		printf("failed to find RPMSG dev idx\n");
		goto err;
	}

	if (snprintf(rpmsg_dev, sizeof(rpmsg_dev), "/dev/rpmsg%d", idx) < 0) {
		printf("failed to compose RPMSG dev path\n");
		goto err;
	}

	fd = open(rpmsg_dev, O_RDWR);
	if (fd < 0)
		printf("failed to open RPMSG dev: %s\n", rpmsg_dev);

	return fd;

err:
	printf("rpmsg_init() failed\n");
	return -1;
}

void rpmsg_deinit(int fd)
{
	close(fd);
}
