# poke-rls-asm.S
# Helper functions for poke-rls.c
# NetWatch system management mode administration console
#
# Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
# This program is free software; you can redistribute and/or modify it under
# the terms found in the file LICENSE in the root of this source tree.

.global poke
poke:
	mov 8(%esp), %ecx

	# Parameter: I/O port to set bit 2 of
	mov 4(%esp), %edx
	inl %dx, %eax
	or $4, %eax
	outl %eax, %dx

	# Maybe SMM poked something?
	mov %ecx, %eax
	ret
