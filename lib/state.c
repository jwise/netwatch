/* state.c
 * SMM saved state manipulation functions.
 * NetWatch system management mode administration console
 *
 * Copyright 2009, Google Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,           
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY           
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "state.h"
#include <cpuid.h>
#include <output.h>
#include <minilib.h>

/* Size flags. */
#define SZ_BYTE		0x10000000
#define SZ_WORD		0x20000000
#define SZ_DWORD	0x40000000
#define SZ_QWORD	0x80000000
#define OFFSET_MASK	0x0FFFFFFF

static const uint32_t offset_table_legacy[] = {
	[STATE_REV]		= 0xfefc | SZ_DWORD,
	[STATE_REG_SMBASE]	= 0xfef8 | SZ_DWORD,
	[STATE_REG_IORESTART]	= 0xff00 | SZ_WORD,
	[STATE_REG_HALTRESTART]	= 0xff02 | SZ_WORD,

	[STATE_REG_EAX]		= 0xffd0 | SZ_DWORD,
	[STATE_REG_EBX]		= 0xffdc | SZ_DWORD,
	[STATE_REG_ECX]		= 0xffd4 | SZ_DWORD,
	[STATE_REG_EDX]		= 0xffd8 | SZ_DWORD,
	[STATE_REG_ESI]		= 0xffe8 | SZ_DWORD,
	[STATE_REG_EDI]		= 0xffec | SZ_DWORD,
	[STATE_REG_ESP]		= 0xffe0 | SZ_DWORD,
	[STATE_REG_EBP]		= 0xffe4 | SZ_DWORD,
	[STATE_REG_EIP]		= 0xfff0 | SZ_DWORD,
	[STATE_REG_EFLAGS]	= 0xfff4 | SZ_DWORD,
	[STATE_REG_CR0]		= 0xfffc | SZ_DWORD,
	[STATE_REG_CR3]		= 0xfff8 | SZ_DWORD,

	[STATE_REG_CS]		= 0xffac | SZ_DWORD,
	[STATE_REG_SS]		= 0xffb0 | SZ_DWORD,
	[STATE_REG_DS]		= 0xffb4 | SZ_DWORD,
	[STATE_REG_ES]		= 0xffa8 | SZ_DWORD,
	[STATE_REG_FS]		= 0xffb8 | SZ_DWORD,
	[STATE_REG_GS]		= 0xffbc | SZ_DWORD,
	[STATE_REG_IDT_BASE]	= 0xff94 | SZ_DWORD
};

#define MAX_REG_LEGACY (sizeof(offset_table_legacy)/sizeof(offset_table_legacy[0]) - 1)

static const uint32_t offset_table_amd64[] = {
	[STATE_REV]		= 0xfefc | SZ_DWORD,
	[STATE_REG_SMBASE]	= 0xff00 | SZ_DWORD,
	[STATE_REG_IORESTART]	= 0xfec8 | SZ_BYTE,
	[STATE_REG_HALTRESTART]	= 0xfec9 | SZ_BYTE,

	[STATE_REG_EAX]		= 0xfff8 | SZ_DWORD,
	[STATE_REG_EBX]		= 0xffe0 | SZ_DWORD,
	[STATE_REG_ECX]		= 0xfff0 | SZ_DWORD,
	[STATE_REG_EDX]		= 0xffe8 | SZ_DWORD,
	[STATE_REG_ESI]		= 0xffc8 | SZ_DWORD,
	[STATE_REG_EDI]		= 0xffc0 | SZ_DWORD,
	[STATE_REG_ESP]		= 0xffd8 | SZ_DWORD,
	[STATE_REG_EBP]		= 0xffd0 | SZ_DWORD,
	[STATE_REG_EIP]		= 0xff78 | SZ_DWORD,
	[STATE_REG_EFLAGS]	= 0xff70 | SZ_DWORD,
	[STATE_REG_CR0]		= 0xff58 | SZ_QWORD,
	[STATE_REG_CR3]		= 0xff50 | SZ_QWORD,

	[STATE_REG_CS]		= 0xfe10 | SZ_WORD,
	[STATE_REG_CS_ATTRIB]	= 0xfe12 | SZ_WORD,
	[STATE_REG_CS_BASE]	= 0xfe18 | SZ_QWORD,
	[STATE_REG_CS_LIMIT]	= 0xfe14 | SZ_DWORD,
	[STATE_REG_SS]		= 0xfe20 | SZ_WORD,
	[STATE_REG_SS_ATTRIB]	= 0xfe22 | SZ_WORD,
	[STATE_REG_SS_BASE]	= 0xfe28 | SZ_QWORD,
	[STATE_REG_SS_LIMIT]	= 0xfe24 | SZ_DWORD,
	[STATE_REG_DS]		= 0xfe30 | SZ_WORD,
	[STATE_REG_DS_ATTRIB]	= 0xfe32 | SZ_WORD,
	[STATE_REG_DS_BASE]	= 0xfe38 | SZ_QWORD,
	[STATE_REG_DS_LIMIT]	= 0xfe34 | SZ_DWORD,
	[STATE_REG_ES]		= 0xfe00 | SZ_WORD,
	[STATE_REG_ES_ATTRIB]	= 0xfe02 | SZ_WORD,
	[STATE_REG_ES_BASE]	= 0xfe08 | SZ_QWORD,
	[STATE_REG_ES_LIMIT]	= 0xfe04 | SZ_DWORD,
	[STATE_REG_FS]		= 0xfe40 | SZ_WORD,
	[STATE_REG_FS_ATTRIB]	= 0xfe42 | SZ_WORD,
	[STATE_REG_FS_BASE]	= 0xfe48 | SZ_QWORD,
	[STATE_REG_FS_LIMIT]	= 0xfe44 | SZ_DWORD,
	[STATE_REG_GS]		= 0xfe50 | SZ_WORD,
	[STATE_REG_GS_ATTRIB]	= 0xfe52 | SZ_WORD,
	[STATE_REG_GS_BASE]	= 0xfe58 | SZ_QWORD,
	[STATE_REG_GS_LIMIT]	= 0xfe54 | SZ_DWORD,
	[STATE_REG_IDT_BASE]	= 0xfe88 | SZ_QWORD,
	[STATE_REG_IDT_LIMIT]	= 0xfe84 | SZ_DWORD,

	[STATE_REG_RAX]		= 0xfff8 | SZ_QWORD,
	[STATE_REG_RBX]		= 0xffe0 | SZ_QWORD,
	[STATE_REG_RCX]		= 0xfff0 | SZ_QWORD,
	[STATE_REG_RDX]		= 0xffe8 | SZ_QWORD,
	[STATE_REG_RSI]		= 0xffc8 | SZ_QWORD,
	[STATE_REG_RDI]		= 0xffc0 | SZ_QWORD,
	[STATE_REG_RSP]		= 0xffd8 | SZ_QWORD,
	[STATE_REG_RBP]		= 0xffd0 | SZ_QWORD,
	[STATE_REG_R8]		= 0xffb8 | SZ_QWORD,
	[STATE_REG_R9]		= 0xffb0 | SZ_QWORD,
	[STATE_REG_R10]		= 0xffa8 | SZ_QWORD,
	[STATE_REG_R11]		= 0xffa0 | SZ_QWORD,
	[STATE_REG_R12]		= 0xff98 | SZ_QWORD,
	[STATE_REG_R13]		= 0xff90 | SZ_QWORD,
	[STATE_REG_R14]		= 0xff88 | SZ_QWORD,
	[STATE_REG_R15]		= 0xff80 | SZ_QWORD,
	[STATE_REG_RIP]		= 0xff78 | SZ_QWORD,
	[STATE_REG_RFLAGS]	= 0xff70 | SZ_QWORD,

	[STATE_REG_EFER]	= 0xfed0 | SZ_QWORD
};

static const char register_names[][4] = {
	[STATE_REV]		= "sREV",
	[STATE_REG_SMBASE]	= "sBSE",
	[STATE_REG_IORESTART]	= "IOrs",
	[STATE_REG_HALTRESTART]	= "HLrs",

	[STATE_REG_EAX]		= "%eax",
	[STATE_REG_EBX]		= "%ebx",
	[STATE_REG_ECX]		= "%ecx",
	[STATE_REG_EDX]		= "%edx",
	[STATE_REG_ESI]		= "%esi",
	[STATE_REG_EDI]		= "%edi",
	[STATE_REG_ESP]		= "%esp",
	[STATE_REG_EBP]		= "%ebp",
	[STATE_REG_EIP]		= "%eip",
	[STATE_REG_EFLAGS]	= "%eFL",
	[STATE_REG_CR0]		= "%cr0",
	[STATE_REG_CR3]		= "%cr3",

	[STATE_REG_CS]		= "%cs ",
	[STATE_REG_CS_ATTRIB]	= "csAT",
	[STATE_REG_CS_BASE]	= "csBA",
	[STATE_REG_CS_LIMIT]	= "csLI",
	[STATE_REG_SS]		= "%ss ",
	[STATE_REG_SS_ATTRIB]	= "ssAT",
	[STATE_REG_SS_BASE]	= "ssBA",
	[STATE_REG_SS_LIMIT]	= "ssLI",
	[STATE_REG_DS]		= "%ds ",
	[STATE_REG_DS_ATTRIB]	= "dsAT",
	[STATE_REG_DS_BASE]	= "dsBA",
	[STATE_REG_DS_LIMIT]	= "dsLI",
	[STATE_REG_ES]		= "%es ",
	[STATE_REG_ES_ATTRIB]	= "esAT",
	[STATE_REG_ES_BASE]	= "esBA",
	[STATE_REG_ES_LIMIT]	= "esLI",
	[STATE_REG_FS]		= "%fs ",
	[STATE_REG_FS_ATTRIB]	= "fsAT",
	[STATE_REG_FS_BASE]	= "fsBA",
	[STATE_REG_FS_LIMIT]	= "fsLI",
	[STATE_REG_GS]		= "%gs ",
	[STATE_REG_GS_ATTRIB]	= "gsAT",
	[STATE_REG_GS_BASE]	= "gsBA",
	[STATE_REG_GS_LIMIT]	= "gsLI",

	[STATE_REG_RAX]		= "%rax",
	[STATE_REG_RBX]		= "%rbx",
	[STATE_REG_RCX]		= "%rcx",
	[STATE_REG_RDX]		= "%rdx",
	[STATE_REG_RSI]		= "%rsi",
	[STATE_REG_RDI]		= "%rdi",
	[STATE_REG_RSP]		= "%rsp",
	[STATE_REG_RBP]		= "%rbp",
	[STATE_REG_R8]		= "%r8 ",
	[STATE_REG_R9]		= "%r9 ",
	[STATE_REG_R10]		= "%r10",
	[STATE_REG_R11]		= "%r11",
	[STATE_REG_R12]		= "%r12",
	[STATE_REG_R13]		= "%r13",
	[STATE_REG_R14]		= "%r14",
	[STATE_REG_R15]		= "%r15",
	[STATE_REG_RIP]		= "%rip",

	[STATE_REG_EFER]	= "EFER"
};

#define MAX_REG_AMD64 (sizeof(offset_table_amd64)/sizeof(offset_table_amd64[0]) - 1)

static enum smm_type smm_type = SMM_TYPE_UNKNOWN;

/* Probe CPUID to figure out what kind of processor this actually is.
 * We memoize this in 'smm_type', so cpuid only needs to happen once.
 */

static void check_smm_type (void) {

	struct cpuid_result r;

	if (smm_type != SMM_TYPE_UNKNOWN)
		return;

	cpuid(0x80000000, &r);

	if (r.eax < 0x80000001) {
		smm_type = SMM_TYPE_32;
		return;
	}

	cpuid(0x80000001, &r);

	if (r.edx & 0x20000000) {
		smm_type = SMM_TYPE_64;
	} else {
		smm_type = SMM_TYPE_32;
	}
}

/* Get the offset of a register, by looking up in the appropriate table.
 */

static unsigned long get_offset(enum state_reg_t reg) {

	check_smm_type();

	if (smm_type == SMM_TYPE_32 && reg <= MAX_REG_LEGACY)
		return offset_table_legacy[reg];
	else if (smm_type == SMM_TYPE_64 && reg <= MAX_REG_AMD64)
		return offset_table_amd64[reg];
	else
		return 0;
}

/* Which variety of processor are we running on?
 */

enum smm_type state_get_type (void) {
	check_smm_type();
	return smm_type;
}

/* Get a register.
 *
 * We assume that Aseg is always direct-mapped at 0xA0000. This may
 * not be the case in the future, with multiple cores, but it is a
 * safe assumption now.
 */

uint64_t state_get_reg (enum state_reg_t reg) {
	unsigned long offset = get_offset(reg);
	void * addr;
	uint64_t value;

	if (!offset)
		return 0;

	addr = (void *)((offset & OFFSET_MASK) + 0xA0000);

	if (offset & SZ_BYTE)
		value = *(uint8_t *)addr;
	else if (offset & SZ_WORD)
		value = *(uint16_t *)addr;
	else if (offset & SZ_DWORD)
		value = *(uint32_t *)addr;
	else
		value = *(uint64_t *)addr;

	return value;
}

/* Get the size of a register, extracted from the saved state offset table.
 */

int state_reg_size (enum state_reg_t reg) {
	unsigned long offset = get_offset(reg);

	if (offset & SZ_BYTE) return 1;
	else if (offset & SZ_WORD) return 2;
	else if (offset & SZ_DWORD) return 4;
	else if (offset & SZ_QWORD) return 8;
	else return 0;
}

/* Modify a saved register.
 *
 * The same caveat about aseg's location applies here as well.
 */

int state_set_reg (enum state_reg_t reg, uint64_t value) {
	unsigned long offset = get_offset(reg);
	void * addr;

	if (!offset)
		return -1;

	addr = (void *)((offset & OFFSET_MASK) + 0xA0000);

	if (offset & SZ_BYTE)
		*(uint8_t *)addr = (uint8_t) value;
	else if (offset & SZ_WORD)
		*(uint16_t *)addr = (uint16_t) value;
	else if (offset & SZ_DWORD)
		*(uint32_t *)addr = (uint32_t) value;
	else
		*(uint64_t *)addr = value;

	return 0;
}

/* Dump the name and contents of a register to a string.
 *
 * Returns: The number of bytes written.
 */

int state_dump_reg(char * dest, int max, enum state_reg_t reg) {
	const char const * name = register_names[reg];
	switch (state_reg_size(reg)) {
	case 1:
		return snprintf(dest, max, "%.4s: 0x%02x\n",
			name, (unsigned int)state_get_reg(reg));
	case 2:
		return snprintf(dest, max, "%.4s: 0x%04x\n",
			name, (unsigned int)state_get_reg(reg));
	case 4:
		return snprintf(dest, max, "%.4s: 0x%08x\n",
			name, (unsigned int)state_get_reg(reg));
	case 8: {
		uint64_t v = state_get_reg(reg);
		return snprintf(dest, max, "%.4s: 0x%08x%08x\n",
			name, (unsigned int)(v>>32), (unsigned int)v);
	}
	default:
		return 0;
	}
}

int state_num_regs() {
	check_smm_type();

	if (smm_type == SMM_TYPE_32)
		return MAX_REG_LEGACY;
	else if (smm_type == SMM_TYPE_64)
		return MAX_REG_AMD64;
	return 0;
}
