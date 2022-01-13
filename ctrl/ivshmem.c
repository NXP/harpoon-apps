/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "ivshmem.h"

#define UIO_MAX_MEM_SIZE	0x1000000	/* Arbitrary size limit */

static int uio_read_mem_size(unsigned int uio_id, int id, size_t *size)
{
	char sysfs_path[64];
	char buf[20];
	int fd, rc;

	if (snprintf(sysfs_path, sizeof(sysfs_path), "/sys/class/uio/uio%u/maps/map%d/size", uio_id, id) < 0)
		goto err;

	fd = open(sysfs_path, O_RDONLY);
	if (fd < 0)
		goto err_open;

	rc = read(fd, buf, sizeof(buf) - 1);
	if (rc < 0)
		goto err_read;

	buf[19] = '\0';

	if (sscanf(buf, "0x%zx", size) < 1)
		goto err_sscanf;

	if (*size > UIO_MAX_MEM_SIZE)
		goto err_range;

	close(fd);

	return 0;

err_range:
err_sscanf:
err_read:
	close(fd);
err_open:
err:
	return -1;
}

void ivshmem_exit(struct ivshmem *mem)
{
	munmap(mem->out, mem->out_size);
	munmap(mem->in, mem->in_size);
	munmap(mem->rw, mem->rw_size);
	munmap(mem->state, mem->state_size);
	munmap(mem->regs, mem->regs_size);

	close(mem->fd);
}

int ivshmem_init(struct ivshmem *mem, unsigned int uio_id)
{
	char uio_path[64];
	int pgsize;
	off_t offset;
	int fd, rc;

	pgsize = getpagesize();

	if (snprintf(uio_path, sizeof(uio_path), "/dev/uio%u", uio_id) < 0)
		goto err;

	mem->fd = open(uio_path, O_RDWR);
	if (mem->fd < 0) {
		fprintf(stderr, "open(%s) failed: %s\n", uio_path, strerror(errno));
		goto err_open;
	}

	offset = 0;
	if (uio_read_mem_size(uio_id, 0, &mem->regs_size) < 0)
		goto err_regs;

	mem->regs = mmap(NULL, mem->regs_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem->fd, offset);
	if (mem->regs == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		goto err_regs;
	}

	offset += pgsize;

	if (uio_read_mem_size(uio_id, 1, &mem->state_size) < 0)
		goto err_state;

	mem->state = mmap(NULL, mem->state_size, PROT_READ, MAP_SHARED, mem->fd, offset);
	if (mem->state == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		goto err_state;
	}

	offset += pgsize;

	if (uio_read_mem_size(uio_id, 2, &mem->rw_size) < 0)
		goto err_rw;

	mem->rw = mmap(NULL, mem->rw_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem->fd, offset);
	if (mem->rw == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		goto err_rw;
	}

	offset += pgsize;

	if (uio_read_mem_size(uio_id, 3, &mem->in_size) < 0)
		goto err_in;

	mem->in = mmap(NULL, mem->in_size, PROT_READ, MAP_SHARED, mem->fd, offset);
	if (mem->in == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		goto err_in;
	}

	offset += pgsize;
	if (uio_read_mem_size(uio_id, 4, &mem->out_size) < 0)
		goto err_out;

	mem->out = mmap(NULL, mem->out_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem->fd, offset);
	if (mem->out == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		goto err_out;
	}

	return 0;

err_out:
	munmap(mem->in, mem->in_size);

err_in:
	munmap(mem->rw, mem->rw_size);

err_rw:
	munmap(mem->state, mem->state_size);

err_state:
	munmap(mem->regs, mem->regs_size);

err_regs:
	close(mem->fd);

err_open:
err:
	mem->fd = -1;

	return -1;
}
