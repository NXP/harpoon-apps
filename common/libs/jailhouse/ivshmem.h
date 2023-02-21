/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _IVSHMEM_H_
#define _IVSHMEM_H_

#define MAX_IV_PEERS	8

struct ivshmem {
	unsigned int peers;
	unsigned int id;
	void *state;
	unsigned int state_size;
	void *rw;
	unsigned int rw_size;
	void *out[MAX_IV_PEERS]; /* array of equal size blocks, one per peer */
	unsigned int out_size;
};

int ivshmem_init(unsigned int bfd, struct ivshmem *ivshmem);
int ivshmem_transport_init(unsigned int bdf, struct ivshmem *mem,
				  void **tp, void **cmd, void **resp);

#endif /* _IVSHMEM_H_ */
