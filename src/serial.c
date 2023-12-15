/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "serial.h"
#include "utils.h"

/*
 * Initialise the serial port.
 *
 * This code comes from seL4 (src/plat/pc99/machine/io.c).
 */
void serial_init(void)
{
	while (!(in8(SERIAL_PORT + 5) & 0x60)) /* wait until not busy */
		;

	out8(SERIAL_PORT + 1, 0x00); /* disable generating interrupts */
	out8(SERIAL_PORT + 3, 0x80); /* line control register: command: set divisor */
	out8(SERIAL_PORT,     0x01); /* set low byte of divisor to 0x01 = 115200 baud */
	out8(SERIAL_PORT + 1, 0x00); /* set high byte of divisor to 0x00 */
	out8(SERIAL_PORT + 3, 0x03); /* line control register: set 8 bit, no parity, 1 stop bit */
	out8(SERIAL_PORT + 4, 0x0b); /* modem control register: set DTR/RTS/OUT2 */

	in8(SERIAL_PORT);     /* clear receiver SERIAL_PORT */
	in8(SERIAL_PORT + 5); /* clear line status SERIAL_PORT */
	in8(SERIAL_PORT + 6); /* clear modem status SERIAL_PORT */
}

/*
 * Print a number in decimal format on the serial port.
 */
void serial_put_uint64(uint64_t num)
{
	char buf[21];
	int i = sizeof (buf);

	if (!num) {
		serial_putc('0');
		return;
	}

	buf[--i] = '\0';
	while (num > 0 && --i > 0) {
		buf[i] = '0' + (num % 10);
		num /= 10;
	}
	serial_puts(&buf[i]);
}
