/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdint.h>

#if ENABLE_STORAGE == 1
int storage_read_ipv4_address(const char *filename, uint8_t *addr);
int storage_read_mac_address(const char *filename, uint8_t *mac);
int storage_read_uint(const char *filename, unsigned int *value);
int storage_read_int(const char *filename, int *value);
int storage_read_float(const char *filename, float *value);
int storage_read_u8(const char *filename, uint8_t *value);
int storage_read_u16(const char *filename, uint16_t *value);
int storage_read_u32(const char *filename, uint32_t *value);
int storage_read_u64(const char *filename, uint64_t *value);
int storage_read_s8(const char *filename, int8_t *value);
int storage_read_s16(const char *filename, int16_t *value);
int storage_read_s32(const char *filename, int32_t *value);

int storage_pwd(void);
int storage_cd(const char *filename);
int storage_ls(void);
int storage_rm(const char *filename);
int storage_mkdir(const char *filename);
int storage_cat(const char *filename);
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
static inline int storage_pwd(void)
{
    return 0;
}
static inline int storage_cd(const char *filename)
{
    return 0;
}
static inline int storage_ls(void)
{
    return 0;
}
static inline int storage_rm(const char *filename)
{
     return 0;
}
static inline int storage_mkdir(const char *filename)
{
    return 0;
}
static inline int storage_cat(const char *filename)
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
