/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <poll.h>
#include <string.h>

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
		if (nr_read <= 0)
			if (!nr_read || errno == EAGAIN)
				break;
			else
				return nr_read;

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
		if (nr_write <= 0)
			if (!nr_write || errno == EAGAIN)
				break;
			else
				return nr_write;

		nr_left -= nr_write;
		pos += nr_write;
	}

	return (len - nr_left);
}

int rpmsg_send(int fd, const void *data, unsigned int len)
{
	int ret;
	int err = 0;

	ret = writen(fd, data, len);
	if (ret != len)
		err = -1;

	return err;
}

int rpmsg_recv(int fd, void *data, unsigned int *len, int timeout)
{
	int ret, err = -1;
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = POLLIN;

	if (poll(&pfd, 1, timeout) < 0) {
		perror("poll()");
		goto out;
	}

	if (pfd.revents & POLLIN) {
		ret = readn(fd, data, *len);
		if (ret >= 0) {
			*len = ret;
			err = 0;
		}
	}
out:
	return err;
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

	/* Add the close-on-exec flag for the rpmsg device file descriptor to avoid keeping it open
	* in child processes launched by the runtime configuration through the system() call.
	* Otherwise, the control application will fail to open the device again.
	*/
	fd = open(rpmsg_dev, O_RDWR | O_NONBLOCK | O_CLOEXEC);

	if (fd < 0)
		printf("failed to open RPMSG dev: %s, errno: %s\n", rpmsg_dev, strerror(errno));

	return fd;

err:
	printf("rpmsg_init() failed\n");
	return -1;
}

void rpmsg_deinit(int fd)
{
	close(fd);
}
