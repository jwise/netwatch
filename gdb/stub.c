/* stub.c
 * GDB stub system manipultion code.
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

#include <stdint.h>
#include <msr.h>
#include <output.h>
#include <state.h>
#include <demap.h>
#include <paging.h>
#include <reg-x86.h>
#include "proto.h"
#include "stub.h"

void send_stop_packet();

uint16_t saved_db_entry;
uint64_t saved_db_entry_location;
enum operating_mode step_start_mode;
uint64_t saved_cs_base;
uint64_t saved_eflags;
uint64_t saved_cs_limit;
uint64_t saved_cs_attrib;
uint64_t saved_ss_base;
uint64_t saved_ss_limit;
uint64_t saved_ss_attrib;

#define P64(x)	(uint32_t)((x)>>32), (uint32_t)(x)

static const enum state_reg_t regs32 [] = {
	STATE_REG_EAX, STATE_REG_ECX, STATE_REG_EDX, STATE_REG_EBX,
	STATE_REG_ESP, STATE_REG_EBP, STATE_REG_ESI, STATE_REG_EDI,
	STATE_REG_EIP, STATE_REG_EFLAGS, STATE_REG_CS, STATE_REG_SS,
	STATE_REG_DS, STATE_REG_ES, STATE_REG_FS, STATE_REG_GS
};

static enum run_mode run_mode;

void set_run_mode(enum run_mode mode) {
	run_mode = mode;
}

int gdb_post_smi_hook() {
	uint64_t r, entry;
	uint32_t * ptr;
	uint16_t * entry_v;
	enum operating_mode m = get_operating_mode();


	switch (run_mode) {
	case RM_STOPPED:
		/* Until we have a better way of handling this - spin in
		 * a loop. */
		return 1;

	case RM_STEPPING:
		/* First, set RFLAGS.TF */
		r = state_get_reg(STATE_REG_EFLAGS);
		saved_eflags = r;
		r |= EFLAGS_TF;
		/* We also don't want to deal with interrupts. */
		r &= ~EFLAGS_IF;
		state_set_reg(STATE_REG_EFLAGS, r);

		/* Track down the handler for a debug exception */
		r = state_get_reg(STATE_REG_IDT_LIMIT);
		r = state_get_reg(STATE_REG_IDT_BASE);
		if (m == LONG_64BIT || m == LONG_COMPAT) {
			/* 64-bit. Index up 16 bytes to get to the
			 * debug trap descriptor. */
			ptr = demap(r + 16);
			entry = (*ptr) & 0xFFFF;
			ptr = demap(r + 20);
			entry |= (*ptr) & 0xFFFF0000;
			ptr = demap(r + 24);
			entry |= (uint64_t)(*ptr) << 32;
		} else {
			/* Assume 32-bit for now. */
			ptr = demap(r + 8);
			entry = (*ptr) & 0xFFFF;
			ptr = demap(r + 12);
			entry |= (*ptr) & 0xFFFF0000;
		}

		outputf("entry is at %08x %08x", (uint32_t)(entry>>32), (uint32_t)entry);
		/* MAGIC_BREAK */
		saved_db_entry_location = demap_phys(entry);
		entry_v = p2v(saved_db_entry_location);
		outputf("entry_v mapped to %08x", entry_v);
		if (!entry_v)  {
			run_mode = RM_UNENCUMBERED; break;
		}
		saved_db_entry = *entry_v;
		*entry_v = 0xB2E6;		/* "out %al, $0xb2" */

		step_start_mode = m;

		/* Turn off the safety */
		WRMSR(0xc0010054, 0x8002);
		outputf("Have fun!");

		saved_cs_base = state_get_reg(STATE_REG_CS_BASE);
		saved_cs_limit = state_get_reg(STATE_REG_CS_LIMIT);
		saved_cs_attrib = state_get_reg(STATE_REG_CS_ATTRIB);
		saved_ss_base = state_get_reg(STATE_REG_SS_BASE);
		saved_ss_limit = state_get_reg(STATE_REG_SS_LIMIT);
		saved_ss_attrib = state_get_reg(STATE_REG_SS_ATTRIB);

		break;

	case RM_CONTINUING:
		/* There may be breakpoints. Fall through to
		 * "unencumbered" for now. */	

	case RM_UNENCUMBERED:
		/* Nada. */
		return 0;

	default:
		break;
	}

	return 0;
}

#define READ_QWORD(pa)	({ uint64_t * p = demap(pa); if (!p) goto fail; outputf("pq %08x %08x", (uint32_t)((*p)>>32), (uint32_t)(*p)); *p; })
#define READ_WORD(pa)	({ uint16_t * p = demap(pa); if (!p) goto fail; outputf("p %04x", (*p)); *p; })

static void gdb_unmangle_stepped_system() {
	uint64_t r;
	uint16_t * entry_v;

	/* The unexpected breakpoint has caused an interrupt stack to be
	 * built, which we must get rid of. */

	if (step_start_mode == LONG_64BIT) {
		r = state_get_reg(STATE_REG_RSP);
		state_set_reg(STATE_REG_RIP, READ_QWORD(r));
		state_set_reg(STATE_REG_CS, READ_WORD(r+8));
		state_set_reg(STATE_REG_RFLAGS, READ_QWORD(r+16));
		state_set_reg(STATE_REG_RSP, READ_QWORD(r+24));
		state_set_reg(STATE_REG_SS, READ_WORD(r+32));
	} else if (step_start_mode == LONG_COMPAT) {
		r = state_get_reg(STATE_REG_RSP);
		state_set_reg(STATE_REG_RIP, READ_QWORD(r) & 0xFFFFFFFF);
		state_set_reg(STATE_REG_CS, READ_WORD(r+8));
		state_set_reg(STATE_REG_RFLAGS, READ_QWORD(r+16) & 0xFFFFFFFF);
		state_set_reg(STATE_REG_RSP, READ_QWORD(r+24) & 0xFFFFFFFF);
		state_set_reg(STATE_REG_SS, READ_WORD(r+32));
	}
		entry_v = p2v(saved_db_entry_location);
		*entry_v = saved_db_entry;

	state_set_reg(STATE_REG_CS_BASE, saved_cs_base);
	state_set_reg(STATE_REG_CS_LIMIT, saved_cs_limit);
	state_set_reg(STATE_REG_CS_ATTRIB, saved_cs_attrib);
	state_set_reg(STATE_REG_SS_BASE, saved_ss_base);
	state_set_reg(STATE_REG_SS_LIMIT, saved_ss_limit);
	state_set_reg(STATE_REG_SS_ATTRIB, saved_ss_attrib);
	state_set_reg(STATE_REG_EFLAGS, saved_eflags);

	/* Put us back in "stopped" mode, until the GDB server gets around
	 * to handling the step.
	 */

	reset_operating_mode_memo();
	run_mode = RM_STOPPED;

	send_stop_packet();
	return;

fail:
	outputf("ERROR: Failed to restore state!");
	while(1);
}

void gdb_pre_smi_hook() {
	enum operating_mode m = get_operating_mode();
	uint64_t ip, break_phys;

	if (run_mode == RM_STEPPING) {
		/* Oh hey, we are probably stopped on a breakpoint.
		 * Let's check. */
		ip = state_get_reg(
			(m == LONG_64BIT)
			? STATE_REG_RIP : STATE_REG_EIP
		);
		break_phys = demap_phys(ip);

		if ((break_phys != saved_db_entry_location)
		 && (break_phys != saved_db_entry_location + 2)) {
			/* Some other event caused us to enter SMM. We'll deal
			 * with the single step when we *actually* get to
			 * the breakpoint. */
			return;
		}

		/* Great. Now we have caused a debug exception.
		 *
		 * "Your problems just got worse. Think: what have you done?"
		 *
		 * We don't want the running system to know that anything
		 * ever happened, so we manually unwind the stack and undo
		 * everything that happened.
		 */

		gdb_unmangle_stepped_system();
	}
}


void read_registers_32 (char * buf) {
	int i, r = 0, offset = 0, size = 0;

	/* Dump registers to buffer. */

	for (i = 0; i < (sizeof(regs32) / sizeof(enum state_reg_t)); i++) {
		enum state_reg_t reg = regs32[i];
		r = state_get_reg(reg);
		//size = state_reg_size(reg);
		size = 4;
		enhexificate(&r, buf + offset, size);
		offset += (size * 2);
	}

	/* XXX: The rest of the buffer "should be" filled with floating point
	   stuff. We'll worry about that later. */
}

void write_registers_32 (char * buf) {
	//int size = 0, i;


	uint32_t *ubuf = (uint32_t *)buf;
	
	state_set_reg(STATE_REG_EAX, ubuf[0]);
	state_set_reg(STATE_REG_ECX, ubuf[1]);
	state_set_reg(STATE_REG_EDX, ubuf[2]);
	state_set_reg(STATE_REG_EBX, ubuf[3]);
	state_set_reg(STATE_REG_ESP, ubuf[4]);
	state_set_reg(STATE_REG_EBP, ubuf[5]);
	state_set_reg(STATE_REG_ESI, ubuf[6]);
	state_set_reg(STATE_REG_EDI, ubuf[7]);

	state_set_reg(STATE_REG_EIP, ubuf[8]);
	state_set_reg(STATE_REG_EFLAGS, ubuf[9]);
	state_set_reg(STATE_REG_CS, ubuf[10]);
	state_set_reg(STATE_REG_SS, ubuf[11]);
	state_set_reg(STATE_REG_DS, ubuf[12]);
	state_set_reg(STATE_REG_ES, ubuf[13]);
	state_set_reg(STATE_REG_FS, ubuf[14]);
	state_set_reg(STATE_REG_GS, ubuf[15]);

	/* XXX: Again, need to deal with floating point. */
}
