/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _JH_CONSOLE_H_
#define _JH_CONSOLE_H_

#include "os/stdint.h"

void jh_putc(int c);
void jh_puts(const char *s);
void jh_put_hex(uint64_t v);
void jh_put_dec(uint64_t v);

#endif /* _JH_CONSOLE_H_ */
