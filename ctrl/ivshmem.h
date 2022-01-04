/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IVSHMEM_H_
#define _IVSHMEM_H_

struct ivshmem {
	int fd;

	void *regs;
	size_t regs_size;

	void *state;
	size_t state_size;

	void *rw;
	size_t rw_size;

	void *in;
	size_t in_size;

	void *out;
	size_t out_size;
};

void ivshmem_exit(struct ivshmem *mem);

int ivshmem_init(struct ivshmem *mem, unsigned int uio_id);

#endif /* _IVSHMEM_H_ */
