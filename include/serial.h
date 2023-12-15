/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "utils.h"

/* Hardcode the first serial port for now. */
#define SERIAL_PORT	0x3f8

/*
 * Output a single character to the serial port.
 */
static inline void serial_putc(uint8_t ch)
{
	while ((in8(SERIAL_PORT + 5) & 0x20) == 0)
		;
	out8(SERIAL_PORT, ch);
}

/*
 * Output a null-terminated string to the serial port. Note that newlines "\n"
 * are transparently converted to "\r\n".
 */
static inline void serial_puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			serial_putc('\r');
		serial_putc(*s++);
	}
}

extern void serial_init(void);
extern void serial_put_uint64(uint64_t num);
