/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "multiboot2.h"

/*
 * Multiboot header.
 */
	.section .multiboot
#define MBT2_SIZE       (__mbi2_end - __mbi2_start)
	.align	8
__mbi2_start:
	.long	MULTIBOOT2_HEADER_MAGIC                 /* magic                */
	.long	0                                       /* architecture (i386)  */
	.long	MBT2_SIZE                               /* header size          */
	.long	-(MULTIBOOT2_HEADER_MAGIC + MBT2_SIZE)  /* checksum             */
	.word	0x0                                     /* end tag: type        */
	.word	0x0                                     /* end tag: flags       */
	.long	0x8                                     /* end tag: size        */
__mbi2_end:

/*
 * Entry point.
 */
	.section .text
	.globl	entry
entry:
	/* Load a stack. */
	leal	stack, %esp

	/* Reset the EFLAGS register. */
	pushl	$0
	popf

	/* Jump straight into the C code. */
	pushl	%ebx	/* multiboot_info_ptr */
	pushl	%eax	/* multiboot_magic    */
	call	main

	/* Reboot the machine. Cheeky but convenient. */
	int	$0x19

	/* Normally not reached. */
halt:	hlt
	jmp	halt

/*
 * Allocate a small stack.
 */
	.section .bss
	.align	16
	.fill	0x1000
stack:
