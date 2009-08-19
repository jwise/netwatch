/* traps.c
 * Trap handling routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <output.h>
#include <io.h>
#include <paging.h>

#include "traps.h"

#define CS_SEGSEL	0x10

asm (
	"fault_wrapper_common:\n"
	"push %eax\n"
        "mov %cr2, %eax\n"
        "push %eax\n"
        "push %esp\n"
        "call die\n"
);

#define FAULT_WRAPPER(func_name, code)        asm (	\
        ".global " #func_name "_wrapper\n"              \
        #func_name "_wrapper:\n"                        \
	"mov $" #code ", %eax\n"			\
	"jmp fault_wrapper_common\n"			\
); void func_name##_wrapper(void);

#define FAULT_WRAPPER1(func_name, code)        asm (           \
        ".global " #func_name "_wrapper\n"              \
        #func_name "_wrapper:\n"                        \
	"mov $0, %eax\n"				\
        "push %eax\n"                                   \
	"mov $" #code ", %eax\n"			\
	"jmp fault_wrapper_common\n"			\
); void func_name##_wrapper(void);

#define WRAPPER_INSTALL(idt, idt4_value, func_name, int_number) {	\
	*(int *)((void *)idt + (8 * int_number)) =	\
		((int)func_name##_wrapper & 0xFFFF) |	\
		(CS_SEGSEL << 16);		\
	*(int *)((void *)idt + (8 * int_number) + 4) =	\
		((int)func_name##_wrapper & 0xFFFF0000) | idt4_value; \
}

/* The 16 bits at offset 4 from the start of an interrupt gate are a
 * bitfield, according to the Intel spec:
 * 15           P - Segment Present - set to 1
 * 14-13        DPL - Descriptor privilege level - set to 00, since we're always in ring 0
 * 12-8         01111 for 32-bit trap gates, 01110 for 32-bit interrupt gates
 * 7-0          Set to 0
 */
#define TRAP		0x8F00

typedef struct trap_t {
	int cr2;
	int fault_code;
        int error_code;
        int eip;
        int cs;
        int eflags;
} trap_t;

static const char const * trapcodes[20] = {
	"Divide by Zero",
	"Debug Trap",
	"NMI",
	"Breakpoint Trap",
	"Overflow Trap",
	"Bound Range Fault",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Overrun",
	"Invalid TSS",
	"Segmnet Not Present",
	"Stack Fault",
	"GPF",
	"Page Fault",
	"Reserved Trap",
	"x87 FPE Pending",
	"Alignment Check",
	"Machine Check",
	"SIMD FPE Fault"
};

void die(struct trap_t * trap) {

	DBG(0x40 + trap->fault_code);

	if (trap->fault_code < 20) {
		outputf("---\n%s", trapcodes[trap->fault_code]);
	} else {
		outputf("---\nUNKNOWN FAULT");
	}

	outputf("Error %08x %%eip %08x", trap->error_code, trap->eip);
	outputf("%%cr2 %08x %%eflags %08x", trap->cr2, trap->eflags);
	while(1) asm("hlt");
}

FAULT_WRAPPER1(fault_divide, 0);
FAULT_WRAPPER1(fault_db, 1);
FAULT_WRAPPER1(fault_nmi, 2);
FAULT_WRAPPER1(fault_3, 3);
FAULT_WRAPPER1(fault_overflow, 4);
FAULT_WRAPPER1(fault_oob, 5);
FAULT_WRAPPER1(fault_opcode, 6);
FAULT_WRAPPER1(fault_enofpu, 7);
FAULT_WRAPPER(double_fault, 8);
FAULT_WRAPPER1(fault_fpu, 9);
FAULT_WRAPPER(fault_invalid_tss, 10);
FAULT_WRAPPER(fault_noseg, 11);
FAULT_WRAPPER(fault_stack, 12);
FAULT_WRAPPER(fault_gp, 13);
FAULT_WRAPPER(fault_page, 14);
FAULT_WRAPPER1(fault_fpe, 16);
FAULT_WRAPPER(fault_ac, 17);
FAULT_WRAPPER1(fault_machine, 18);
FAULT_WRAPPER1(fault_other, 19);

/* pseudo_descriptor and x86_gate structs from 15-410 basis code. */

struct x86_gate {
	unsigned int filler[2];		//64 bits, or 8 bytes.
};

static struct x86_gate idt[64];

struct pseudo_descriptor {
        short pad;
        unsigned short limit;
        unsigned long linear_base;
} __attribute__((packed));

void traps_install(void) {
        DBG(0xCA);
	int i;

        struct pseudo_descriptor pdesc;
	pdesc.limit = sizeof(idt) - 1;
	pdesc.linear_base = (int)&idt; //v2p(&idt);

       	DBG(0xCB);

	for (i = 0; i < 32; i++)
		WRAPPER_INSTALL(idt, TRAP, fault_other, i);

	WRAPPER_INSTALL(idt, TRAP, fault_divide, T_DIVIDE_ERROR);
	WRAPPER_INSTALL(idt, TRAP, fault_db, T_DEBUG);
	WRAPPER_INSTALL(idt, TRAP, fault_nmi, T_NMI);
	WRAPPER_INSTALL(idt, TRAP, fault_3, T_INT3);
	WRAPPER_INSTALL(idt, TRAP, fault_overflow, T_OVERFLOW);
	WRAPPER_INSTALL(idt, TRAP, fault_oob, T_OUT_OF_BOUNDS);
	WRAPPER_INSTALL(idt, TRAP, fault_opcode, T_INVALID_OPCODE);
	WRAPPER_INSTALL(idt, TRAP, fault_enofpu, T_NO_FPU);
	WRAPPER_INSTALL(idt, TRAP, double_fault, T_DOUBLE_FAULT);
	WRAPPER_INSTALL(idt, TRAP, fault_fpu, T_FPU_FAULT);
	WRAPPER_INSTALL(idt, TRAP, fault_invalid_tss, T_INVALID_TSS);
	WRAPPER_INSTALL(idt, TRAP, fault_noseg, T_SEGMENT_NOT_PRESENT);
	WRAPPER_INSTALL(idt, TRAP, fault_stack, T_STACK_FAULT);
	WRAPPER_INSTALL(idt, TRAP, fault_gp, T_GENERAL_PROTECTION);
	WRAPPER_INSTALL(idt, TRAP, fault_page, T_PAGE_FAULT);
	WRAPPER_INSTALL(idt, TRAP, fault_fpe, T_FLOATING_POINT_ERROR);
	WRAPPER_INSTALL(idt, TRAP, fault_ac, T_ALIGNMENT_CHECK);
	WRAPPER_INSTALL(idt, TRAP, fault_machine, T_MACHINE_CHECK);

        DBG(0xCC);
	asm volatile("lidt %0" : : "m" (pdesc.limit));
        DBG(0xCE);
}
