#include <output.h>
#include <io.h>
#include <paging.h>

#include "traps.h"

#define CS_SEGSEL	0x10

#define FAULT_WRAPPER(func_name)        asm (           \
        ".global " #func_name "_wrapper\n"              \
        #func_name "_wrapper:\n"                        \
        "mov %cr2, %eax\n"                              \
        "push %eax\n"                                   \
        "push %esp\n"                                   \
        "call " #func_name "\n"                         \
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
 * 14-13        DPL - Descriptor privilege level - set to 11 for trap/int, 00 for IPI
 * 12-8         01111 for 32-bit trap gates, 01110 for 32-bit interrupt gates
 * 7-0          Set to 0
 * Trap:      binary 11101111 0000000, hex EF00.
 * Interrupt: binary 11101110 0000000, hex EE00.
 */
#define TRAP		0xEF00
#define INTERRUPT	0xEE00

typedef struct trap_t {
	int cr2;
        int error_code;
        int eip;
        int cs;
        int eflags;
        int esp;
        int ss;
} trap_t;

void die(struct trap_t * trap) {
	outputf("Error %08x %%eip %08x", trap->error_code, trap->eip);
	outputf("%%esp %08x %%eflags %08x", trap->esp, trap->eflags);
	while(1) asm("hlt");
}

void fault_gp(struct trap_t * trap) {
	outputf("GENERAL PROTECTION FAULT");
	die(trap);
}

void fault_page(struct trap_t * trap) {
	outputf("PAGE FAULT: %08x", trap->cr2);
	die(trap);
}
void fault_divide(struct trap_t * trap) {
	outputf("DIVISION FAULT");
	die(trap);
}
void double_fault(struct trap_t * trap) {
	outputf("DOUBLE FAULT");
	die(trap);
}

FAULT_WRAPPER(fault_gp);
FAULT_WRAPPER(fault_page);
FAULT_WRAPPER(fault_divide);
FAULT_WRAPPER(double_fault);

/* pseudo_descriptor and x86_gate structs from 15-410 basis code. */

struct x86_gate {
unsigned int filler[2]; //64 bits; or 8 bytes.
};

static struct x86_gate idt[64];

struct pseudo_descriptor {
        short pad;
        unsigned short limit;
        unsigned long linear_base;
};

void traps_install(void) {

        struct pseudo_descriptor pdesc;
	pdesc.limit = sizeof(idt) - 1;
	pdesc.linear_base = memory_v2p(&idt);

	WRAPPER_INSTALL(idt, TRAP, fault_divide, T_DIVIDE_ERROR);
	WRAPPER_INSTALL(idt, TRAP, fault_gp, T_GENERAL_PROTECTION);
	WRAPPER_INSTALL(idt, TRAP, fault_page, T_PAGE_FAULT);
	WRAPPER_INSTALL(idt, TRAP, double_fault, T_DOUBLE_FAULT);

        outb(0x80, 0xCC);
	asm volatile("lidt %0" : : "m" (pdesc.limit));
        outb(0x80, 0xCD);
}
