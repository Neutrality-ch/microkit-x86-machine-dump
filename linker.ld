/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

ENTRY(entry)

SECTIONS
{
	. = 1M;

	.text :
	{
		*(.multiboot)
		*(.text*)
		*(.rodata)
	}

	. = ALIGN (CONSTANT (COMMONPAGESIZE));

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(.bss)
	}
}
