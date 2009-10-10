/* mbentry.S
 * Multiboot header
 * NetWatch multiboot loader
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

.globl _start

_start:
	jmp multiboot_entry

	.align 4
multiboot_header:
	.long 0x1badb002	/* Magic value */
	.long 0x00000001	/* Flags -- "Please align modules for me" */
	.long 0xE4524FFD	/* Checksum */

multiboot_entry:
	mov $_stack_top, %esp
	pushl %ebx	/* Multiboot info structure */
	pushl %eax	/* Magic number */
	call c_start
hang:	jmp hang

	.section .bss
	.align 4
_stack:
	.space 16384
_stack_top:
