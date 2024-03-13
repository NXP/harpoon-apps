/*
 * Copyright 2020, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdbool.h>
#include <stdint.h>

#if ENABLE_STORAGE == 1

#include "lfs.h"

#define MAX_FILE_SIZE          32
#define MAX_FILENAME_LENGTH    32
#define MAX_PWD_LENGTH         128
#define MAX_PATH_LENGTH        (MAX_PWD_LENGTH + MAX_FILENAME_LENGTH)

int storage_read_ipv4_address(const char *filename, uint8_t *addr);
int storage_read_mac_address(const char *filename, uint8_t *mac);

int storage_read_qbv_entry(const char *filename, uint8_t *mask, uint32_t *interval, uint8_t *state);

int storage_read_uint(const char *filename, unsigned int *value);
int storage_read_int(const char *filename, int *value);
int storage_read_float(const char *filename, float *value);
int storage_read_bool(const char *filename, bool *value);
int storage_read_u8(const char *filename, uint8_t *value);
int storage_read_u16(const char *filename, uint16_t *value);
int storage_read_u32(const char *filename, uint32_t *value);
int storage_read_u64(const char *filename, uint64_t *value);
int storage_read_s8(const char *filename, int8_t *value);
int storage_read_s16(const char *filename, int16_t *value);
int storage_read_s32(const char *filename, int32_t *value);
int storage_write_uint_hex(const char *filename, unsigned int value);
int storage_write_uint(const char *filename, unsigned int value);
int storage_write_u64(const char *filename, uint64_t value);
int storage_get(const char *dirname, unsigned int n, struct lfs_info *file_info);
int storage_pwd(void);
int storage_cd(const char *filename, bool quiet);
int storage_ls(const char *filename);
int storage_rm(const char *filename, bool recursive, bool force);
int storage_mkdir(const char *dirname, bool parent);
int storage_cat(const char *filename);
int storage_read(const char *filename, char *buf, unsigned int len);
int storage_write(const char *filename, const char *buf, unsigned int len);
int storage_init(void);
int storage_set_shell(void *shell);
void storage_exit(void);
#else
static inline int storage_read_ipv4_address(const char *filename, uint8_t *addr)
{
    return 0;
}
static inline int storage_read_mac_address(const char *filename, uint8_t *mac)
{
    return 0;
}
static inline int storage_read_qbv_entry(const char *filename, uint8_t *mask, uint32_t *offset, uint8_t *state)
{
    return 0;
}
static inline int storage_read_uint(const char *filename, unsigned int *value)
{
    return 0;
}
static inline int storage_read_int(const char *filename, int *value)
{
    return 0;
}
static inline int storage_read_float(const char *filename, float *value)
{
    return 0;
}
static inline int storage_read_bool(const char *filename, bool *value)
{
    return 0;
}
static inline int storage_read_u8(const char *filename, uint8_t *value)
{
    return 0;
}
static inline int storage_read_u16(const char *filename, uint16_t *value)
{
    return 0;
}
static inline int storage_read_u32(const char *filename, uint32_t *value)
{
    return 0;
}
static inline int storage_read_u64(const char *filename, uint64_t *value)
{
    return 0;
}
static inline int storage_read_s8(const char *filename, int8_t *value)
{
    return 0;
}
static inline int storage_read_s16(const char *filename, int16_t *value)
{
    return 0;
}
static inline int storage_read_s32(const char *filename, int32_t *value)
{
    return 0;
}
static inline int storage_write_uint_hex(const char *filename, unsigned int value)
{
    return 0;
}
static inline int storage_write_uint(const char *filename, unsigned int value)
{
    return 0;
}
static inline int storage_write_u64(const char *filename, uint64_t value)
{
    return 0;
}
static inline int storage_get(const char *dirname, unsigned int n, void *file_info)
{
    return 0;
}
static inline int storage_pwd(void)
{
    return 0;
}
static inline int storage_cd(const char *filename, bool quiet)
{
    return 0;
}
static inline int storage_ls(const char *filename)
{
    return 0;
}
static inline int storage_rm(const char *filename, bool recursive, bool force)
{
     return 0;
}
static inline int storage_mkdir(const char *dirname, bool parent)
{
    return 0;
}
static inline int storage_cat(const char *filename)
{
    return 0;
}
static inline int storage_read(const char *filename, char *buf, unsigned int len)
{
    return 0;
}
static inline int storage_write(const char *filename, const char *buf, unsigned int len)
{
    return 0;
}
static inline int storage_init(void)
{
    return 0;
}
static inline int storage_set_shell(void *shell)
{
    return 0;
}
static inline void storage_exit(void)
{
    return;
}

#endif

#endif /* _STORAGE_H_ */
