/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/mmu.h"

#include "ivshmem.h"
#include "memory.h"
#include "hlog.h"
#include "rtos_abstraction_layer.h"

/* Jailhouse COMM */
struct jailhouse_console {
	uint64_t address;
	uint32_t size;
	uint16_t type;
	uint16_t flags;
	uint32_t divider;
	uint32_t gate_nr;
	uint64_t clock_reg;
} __attribute__((packed));

/* from include/jailhouse/hypercall.h */
struct comm_region {
	/** Communication region magic JHCOMM */
	char signature[6];
	/** Communication region ABI revision */
	uint16_t revision;
	/** Cell state, initialized by hypervisor, updated by cell. */
	volatile uint32_t cell_state;
	/** Message code sent from hypervisor to cell. */
	volatile uint32_t msg_to_cell;
	/** Reply code sent from cell to hypervisor. */
	volatile uint32_t reply_from_cell;
	/** Holds static flags, see JAILHOUSE_COMM_FLAG_*. */
	uint32_t flags;
	/** Debug console that may be accessed by the inmate. */
	struct jailhouse_console console;
	/** Base address of PCI memory mapped config. */
	uint64_t pci_mmconfig_base;
};

/* IVSHMEM PCI definitions */
/* PCI Type 0 header */
#define PCI_CFG_VENDOR_ID	0x0
#define PCI_CFG_DEVICE_ID	0x2
#define PCI_CFG_CMD		0x4
#define PCI_CFG_STATUS		0x6

#define PCI_CFG_BAR0		0x10
#define PCI_CFG_CAP		0x34

/* PCI CMD */
#define PCI_CMD_MEM		0x2

/* IVSHMEM Capility */
#define PCI_CAP_IVSHMEM		0x9

#define IVSHMEM_CAP_STATE_SIZE	4
#define IVSHMEM_CAP_RW_SIZE	8
#define IVSHMEM_CAP_OUT_SIZE	16
#define IVSHMEM_CAP_ADDR	24

/* MMIO registers */
#define IVSHMEM_REG_ID                  0x00
#define IVSHMEM_REG_MAX_PEERS           0x04
#define IVSHMEM_REG_INT_CTRL            0x08
#define IVSHMEM_REG_DOORBELL            0x0c
#define IVSHMEM_REG_STATE               0x10

static uint32_t mmio_read32(void *base, unsigned int offset)
{
	return *((volatile uint32_t *)((uintptr_t)base + offset));
}

static uint16_t mmio_read16(void *base, unsigned int offset)
{
	return *((volatile uint16_t *)((uintptr_t)base + offset));
}

static uint8_t mmio_read8(void *base, unsigned int offset)
{
	return *((volatile uint8_t *)((uintptr_t)base + offset));
}

static void mmio_write32(void *base, unsigned int offset, uint32_t val)
{
	*((volatile uint32_t *)((uintptr_t)base + offset)) = val;
}

static void mmio_write16(void *base, unsigned int offset, uint16_t val)
{
	*((volatile uint16_t *)((uintptr_t)base + offset)) = val;
}

static void mmio_write8(void *base, unsigned int offset, uint8_t val)
{
	*((volatile uint8_t *)((uintptr_t)base + offset)) = val;
}

static uint32_t pci_read_config(void *addr, unsigned int offset, unsigned int size)
{
	switch (size) {
	case 1:
		return mmio_read8(addr, offset);
	case 2:
		return mmio_read16(addr, offset);
	case 4:
	default:
		return mmio_read32(addr, offset);
	}
}

static void pci_write_config(void *addr, unsigned int offset, uint32_t val, unsigned int size)
{
	switch (size) {
	case 1:
		mmio_write8(addr, offset, val);
		break;
	case 2:
		mmio_write16(addr, offset, val);
		break;
	case 4:
	default:
		mmio_write32(addr, offset, val);
		break;
	}
}

static uint64_t pci_read_config64(void *addr, unsigned int offset)
{
	return pci_read_config(addr, offset, 4) | ((uint64_t)pci_read_config(addr, offset + 4, 4) << 32);
}

static void *pci_get_device(void *cfg_base, unsigned int bfd)
{
	return (void *)((uintptr_t)cfg_base + (bfd << 12));
}

static void *pci_find_cap(void *base, uint8_t next, uint8_t id)
{
	while (next) {
		if (pci_read_config(base, next, 1) == id)
			return (void *)((uintptr_t)base + next);

		next = pci_read_config(base, next + 1, 1);
	}

	return NULL;
}

int ivshmem_init(unsigned int bfd, struct ivshmem *ivshmem)
{
	struct comm_region *comm;
	void *cfg_base;
	void *pci;
	void *cap;
	void *mmio;
	uint8_t next_cap;
	uintptr_t next_addr, state;
	int ret, i;

	ret = os_mmu_map("jh communication", (uint8_t **)&comm,
			(uintptr_t)HYPERVISOR_COMM_BASE, KB(4),
			OS_MEM_CACHE_WB | OS_MEM_PERM_RW);
	if (ret < 0)
		goto err;

	ret = os_mmu_map("pci cfg", (uint8_t **)&cfg_base,
			(uintptr_t)(comm->pci_mmconfig_base), KB(1024),
			OS_MEM_CACHE_WB | OS_MEM_PERM_RW);
	if (ret < 0)
		goto err;

	pci = pci_get_device(cfg_base, bfd);

	next_cap = pci_read_config(pci, PCI_CFG_CAP, 1);
	cap = pci_find_cap(pci, next_cap, PCI_CAP_IVSHMEM);
	if (!cap)
		goto err;

	/* Update device BAR0 with our MMIO address */
	pci_write_config(pci, PCI_CFG_BAR0, PCI_MMIO_BASE, 4);

	ret = os_mmu_map("pci mmio", (uint8_t **)&mmio,
			(uintptr_t)PCI_MMIO_BASE, KB(4),
			OS_MEM_CACHE_WB | OS_MEM_PERM_RW);
	if (ret < 0)
		goto err;

	/* Tell hypervisor to update */
	pci_write_config(pci, PCI_CFG_CMD, PCI_CMD_MEM, 2);

	/* Find device in PCI configuration */
	ivshmem->id = mmio_read32(mmio, IVSHMEM_REG_ID);
	ivshmem->peers = mmio_read32(mmio, IVSHMEM_REG_MAX_PEERS);
	ivshmem->state_size = pci_read_config(cap, IVSHMEM_CAP_STATE_SIZE, 4);
	ivshmem->rw_size = pci_read_config64(cap, IVSHMEM_CAP_RW_SIZE);
	ivshmem->out_size = pci_read_config64(cap, IVSHMEM_CAP_OUT_SIZE);

	state = (uintptr_t)pci_read_config64(cap, IVSHMEM_CAP_ADDR);
	next_addr = state + ivshmem->state_size;

	ret = os_mmu_map("ivshmem state", (uint8_t **)&ivshmem->state,
			state, ivshmem->state_size,
			OS_MEM_CACHE_WB);
	if (ret < 0)
		goto err;

	if (ivshmem->rw_size) {
		ret = os_mmu_map("ivshmem rw", (uint8_t **)&ivshmem->rw,
				(uintptr_t)next_addr, ivshmem->state_size,
				OS_MEM_CACHE_WB | OS_MEM_PERM_RW);
		if (ret < 0)
			goto err;

		next_addr += ivshmem->rw_size;
	} else {
		ivshmem->rw = NULL;
	}

	if (ivshmem->out_size) {
		rtos_assert(ivshmem->peers <= MAX_IV_PEERS,
				"IVSHMEM peers count(%d) exceed limiation(%d)",
				ivshmem->peers, MAX_IV_PEERS);
		for (i = 0; i < ivshmem->peers; i++) {
			next_addr += i * ivshmem->out_size;

			if (i == ivshmem->id)
				ret = os_mmu_map("ivshmem out",
					(uint8_t **)&ivshmem->out[i],
					next_addr, ivshmem->out_size,
					OS_MEM_CACHE_WB | OS_MEM_PERM_RW);
			else
				ret = os_mmu_map("ivshmem in",
					(uint8_t **)&ivshmem->out[i],
					next_addr, ivshmem->out_size,
					OS_MEM_CACHE_WB);

			if (ret < 0)
				goto err;
		}
	} else {
		for (i = 0; i < ivshmem->peers; i++) {
			ivshmem->out[i] = NULL;
		}
	}

	return 0;

err:
	log_err("ivshmem init failed\n");

	return -1;
}

int ivshmem_transport_init(unsigned int bdf, struct ivshmem *mem,
				  void **tp, void **cmd, void **resp)
{
	int rc;

	if (!mem) {
		mem = rtos_malloc(sizeof(*mem));
		rtos_assert(mem, "malloc for ivshmem struct faild, cannot proceed\n");
	}

	rc = ivshmem_init(bdf, mem);
	rtos_assert(!rc, "ivshmem initialization failed, can not proceed\n");

	rtos_assert(mem->out_size, "ivshmem mis-configuration, can not proceed\n");

	*cmd = mem->out[0];
	*resp = mem->out[mem->id];
	*tp = NULL;

	return 0;
}