/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

void jh_putc(int c)
{
	register uint64_t x1 asm("x1") = c;

	asm volatile(
		"mov x0, #8 \n\t"
		"hvc #0x4a48"
		:
		: "r" (x1)
		: "x0", "memory");
}

void jh_puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			jh_putc('\r');

		jh_putc(*s);

		s++;
	}
}

static const char *hex2str(uint64_t v, char *buf)
{
	static const char *hex_char = "0123456789abcdef";
	int i;

	buf[16] = '\0';
	for (i = 0; i < 16; i++) {
		buf[15 - i] = hex_char[v & 0xf];
		v >>= 4;
	}

	return buf;
}

static const char *dec2str(uint64_t v, char *buf)
{
	buf += 20;
	*buf = '\0';
	do {
		buf--;
		*buf = '0' + v % 10;
		v /= 10;
	} while (v);

	return buf;
}

void jh_put_hex(uint64_t v)
{
	char buf[17];

	jh_puts(hex2str(v, buf));
}

void jh_put_dec(uint64_t v)
{
	char buf[21];

	jh_puts(dec2str(v, buf));
}

