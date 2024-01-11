/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "options.h"
#include "utils.h"

/*
 * Output a single character to the serial port. Note that newlines "\n" are
 * transparently converted to "\r\n".
 */
static inline void serial_putc(uint8_t ch)
{
	if (ch == '\n')
		serial_putc('\r');

	while ((in8(options.serial_port + 5) & 0x20) == 0)
		;
	out8(options.serial_port, ch);
}

/*
 * Output a null-terminated string to the serial port.
 */
static inline void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

extern void serial_init(void);
extern void serial_printf(const char *fmt, ...);
