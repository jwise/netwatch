# pagingstub-asm.s
# Paging-enable helper routines
# NetWatch system management mode administration console
#
# Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
# This program is free software; you can redistribute and/or modify it under
# the terms found in the file LICENSE in the root of this source tree.

.globl set_cr0

set_cr0:
  mov 4(%esp), %eax                     # Load argument
  movl %eax, %cr0                       # Set CR0
  pushw %cs                             # Push 16 bits of %CS
  pushl $set_cr0_cont                   # Push 32 bits of %EIP
  ljmp *(%esp)                          # "Far jump" (both %CS and %EIP from stack)
set_cr0_cont:                           # Hey, here we are!
  addl $6, %esp                         # Un-do 48 bits of pushing
  ret


.globl ps_switch_stack
ps_switch_stack:
  mov 4(%esp), %eax
  mov 8(%esp), %esp   
  call *%eax
  rsm
