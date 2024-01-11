/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef NULL
# define NULL	((void *) 0)
#endif

static inline char memcmp(char *m1, char *m2, uint32_t len)
{
	while (len--)
		if (*m2 != *m1)
			return 1;
	return 0;
}

static inline void memcpy(char *dst, char *src, uint32_t len)
{
	while (len--)
		*dst++ = *src++;
}

static inline void memset(char *mem, char ch, uint32_t len)
{
	while (len--)
		*mem++ = ch;
}

static inline uint32_t strlen(char *str)
{
	uint32_t len = 0;
	while (*str++)
		len++;
	return len;
}

static inline char strcmp(char *s1, char *s2)
{
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s2 - *s1;
}

static inline bool strtou64(char *str, uint64_t *val)
{
	uint64_t num = 0;
	int base = 10;

	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		base = 16;
		str += 2;
	}

	for (; *str; str++) {
		num *= base;
		if (*str >= '0' && *str <= '9')
			num += *str - '0';
		else if (base == 16 && (*str >= 'a' && *str <= 'f'))
			num += *str - 'a' + 10;
		else if (base == 16 && (*str >= 'A' && *str <= 'F'))
			num += *str - 'A' + 10;
		else
			return false;
	}
	*val = num;
	return true;
}

static inline uint8_t in8 (uint16_t port)
{
	uint8_t value;
	__asm__ __volatile__ ("inb %w1,%0":"=a" (value):"Nd" (port));
	return value;
}

static inline void out8 (uint16_t port, uint8_t value)
{
	__asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}
