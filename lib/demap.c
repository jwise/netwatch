/* demap.h
 * Paging lookup functions.
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

#include <state.h>
#include <msr.h>
#include <reg-k8-msr.h>
#include <reg-x86.h>
#include <paging.h>
#include <output.h>
#include <demap.h>

#define REG_CS_ATTRIB_L		(1<<9)
#define PTE_PRESENT		(1<<0)
#define PTE_LARGE		(1<<7)

#define PTE_FOR(x)      (((unsigned int)(x) >> 12) & 0x3FF)
#define PDE_FOR(x)      ((unsigned int)(x) >> 22)
#define ADDR_12_MASK(x) ((unsigned int)(x) & ~((1 << 12) - 1))

/* Keep a memoized copy of the operating mode the CPU was in when we
 * entered SMM, so we don't have to look it up every time.
 */

static enum operating_mode mode;

void reset_operating_mode_memo() {
	mode = UNKNOWN;
}

/* Check the saved state map to determine what mode we were in. */

void probe_operating_mode() {
	unsigned long cr0;

	if (state_get_type() == SMM_TYPE_64) {
		/* This is a 64-bit processor, so we may be in 64-bit
		 * full or compatibility mode. Check if we are. */

		uint64_t efer = state_get_reg(STATE_REG_EFER);

		if (efer & EFER_MSR_LMA) {
			/* We are in long mode. Is this full 64-bit, or
			 * comatibility mode? Check the "L" bit of the
			 * saved CS descriptor to be sure. */

			if (state_get_reg(STATE_REG_CS_ATTRIB)
			    & REG_CS_ATTRIB_L) {
				mode = LONG_64BIT;
			} else {
				mode = LONG_COMPAT;
			}

			return;
		}
		/* Otherwise, we are in legacy mode, so do the normal
		 * 32-bit probes. */
	}

	/* Either we are on a 32-bit processor, or we are on a 64-bit
	 * processor in legacy mode. */

	if (state_get_reg(STATE_REG_EFLAGS) & EFLAGS_VM) {
		mode = V8086;
		return;
	}

	cr0 = state_get_reg(STATE_REG_CR0);

	if (cr0 & CR0_PE)
		if (cr0 & CR0_PG)
			mode = PROTECTED_PAGING;
		else
			mode = PROTECTED_NOPAGING;
	else
		mode = REAL;
}

enum operating_mode get_operating_mode() {
	if (mode == UNKNOWN)
		probe_operating_mode();

	return mode;
}

#define LONG_ADDR_MASK	0x000FFFFFFFFFF000
#define PAGE_1G_MASK	0x000FFFFFC0000000
#define PAGE_2M_MASK	0x000FFFFFFFE00000

#define LONG_ADDR_SECTION(addr, offset)	((((addr) >> (offset)) & 0x1FF) * 8)

#define READ_PHYS_QWORD(pa)	({ uint64_t * p = p2v(pa); if (!p) return 0; *p; })

/* Given a virtual address from the current CPU context, determine what the
 * actual corresponding physical address would be, then convert that back
 * to a virtual address suitable for use within NetWatch.
 *
 * If the given address is not mapped in to RAM or is mapped to RAM which
 * cannot be accessed, returns null.
 *
 * XXX: This currently handles both long and 32-bit modes, but only knows
 * how to parse standard 4-kbyte pages. It assumes all segments span the full
 * 32-bit address space. Thus, it will return correct results for most
 * userspace environments in most sane OS kernels, but not necessarily kernel
 * space (likely to be mapped with large pages) or anything that plays tricks
 * with segmentation (like NaCl).
 */

uint64_t demap_phys (uint64_t vaddr) {

	uint64_t pa = state_get_reg(STATE_REG_CR3) & LONG_ADDR_MASK;
	uint64_t entry;

	if (mode == UNKNOWN)
		probe_operating_mode();

outputf("demapping %08x %08x m %d", (uint32_t)(vaddr>>32), (uint32_t)vaddr, mode);
	switch (mode) {
	case LONG_64BIT:
	case LONG_COMPAT: {
		/* Get PML4 entry */
		entry = READ_PHYS_QWORD(pa + LONG_ADDR_SECTION(vaddr, 39));
		if (!(entry & PTE_PRESENT)) return 0;
		pa = entry & LONG_ADDR_MASK;

		/* Get PDP entry */
		entry = READ_PHYS_QWORD(pa + LONG_ADDR_SECTION(vaddr, 30));
		if (!(entry & PTE_PRESENT)) return 0;
		pa = entry & LONG_ADDR_MASK;

		if (entry & PTE_LARGE)
			return (entry & PAGE_1G_MASK) + (vaddr & 0x3FFFFFFF);

		/* Get PDE */
		entry = READ_PHYS_QWORD(pa + LONG_ADDR_SECTION(vaddr, 21));
		if (!(entry & PTE_PRESENT)) return 0;
		pa = entry & LONG_ADDR_MASK;

		if (entry & PTE_LARGE)
			return (entry & PAGE_2M_MASK) + (vaddr & 0x1FFFFF);

		/* Get PTE */
		entry = READ_PHYS_QWORD(pa + LONG_ADDR_SECTION(vaddr, 12));
		if (!(entry & PTE_PRESENT)) return 0;
		pa = entry & LONG_ADDR_MASK;

		return pa + (vaddr & 0xFFF);
	}
	case PROTECTED_NOPAGING:
	case REAL:
		return vaddr;

	default: {
		unsigned long pde = ((unsigned long *)p2v(pa))[PDE_FOR(vaddr)];
		unsigned long pte;

		if (!(pde & PTE_PRESENT)) return 0; 
		pte = ((unsigned long *)p2v(ADDR_12_MASK(pde)))[PTE_FOR(vaddr)];
		if (!(pte & PTE_PRESENT)) return 0;

		return (pte & ~0xFFF) + (vaddr & 0xFFF);
	}
	}
}

void *demap(uint64_t vaddr) {
	uint64_t paddr = demap_phys(vaddr);
	outputf("demap: paddr 0x%08x %08x", (uint32_t)(paddr>>32), (uint32_t)paddr);
	if (!paddr) return 0;
	return p2v(paddr);
}

