/* state.h
 * Definitions for SMM saved-state manipulation functions.
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

#ifndef __STATE_H
#define __STATE_H

#include <stdint.h>

enum state_reg_t {
	/* Common registers */
	STATE_REV,
	STATE_REG_SMBASE,
	STATE_REG_IORESTART,
	STATE_REG_HALTRESTART,

	/* 32-bit registers (still available in 64-bit mode) */
	STATE_REG_EAX,
	STATE_REG_EBX,
	STATE_REG_ECX,
	STATE_REG_EDX,
	STATE_REG_ESI,
	STATE_REG_EDI,
	STATE_REG_ESP,
	STATE_REG_EBP,
	STATE_REG_EIP,
	STATE_REG_EFLAGS,
	STATE_REG_CR0,
	STATE_REG_CR3,

	STATE_REG_CS,
	STATE_REG_CS_ATTRIB,
	STATE_REG_CS_BASE,
	STATE_REG_CS_LIMIT,
	STATE_REG_SS,
	STATE_REG_SS_ATTRIB,
	STATE_REG_SS_BASE,
	STATE_REG_SS_LIMIT,
	STATE_REG_DS,
	STATE_REG_DS_ATTRIB,
	STATE_REG_DS_BASE,
	STATE_REG_DS_LIMIT,
	STATE_REG_ES,
	STATE_REG_ES_ATTRIB,
	STATE_REG_ES_BASE,
	STATE_REG_ES_LIMIT,
	STATE_REG_FS,
	STATE_REG_FS_ATTRIB,
	STATE_REG_FS_BASE,
	STATE_REG_FS_LIMIT,
	STATE_REG_GS,
	STATE_REG_GS_ATTRIB,
	STATE_REG_GS_BASE,
	STATE_REG_GS_LIMIT,
	STATE_REG_IDT_BASE,
	STATE_REG_IDT_LIMIT,

	/* 64-bit registers */
	STATE_REG_RAX,
	STATE_REG_RBX,
	STATE_REG_RCX,
	STATE_REG_RDX,
	STATE_REG_RSI,
	STATE_REG_RDI,
	STATE_REG_RSP,
	STATE_REG_RBP,
	STATE_REG_R8,
	STATE_REG_R9,
	STATE_REG_R10,
	STATE_REG_R11,
	STATE_REG_R12,
	STATE_REG_R13,
	STATE_REG_R14,
	STATE_REG_R15,
	STATE_REG_RIP,
	STATE_REG_RFLAGS,

	STATE_REG_EFER,

	NUM_REGISTERS
};


enum smm_type {
        SMM_TYPE_UNKNOWN,
        SMM_TYPE_32,
        SMM_TYPE_64
};

enum smm_type state_get_type(void);

uint64_t state_get_reg (enum state_reg_t reg);
int state_reg_size (enum state_reg_t reg);
int state_set_reg (enum state_reg_t reg, uint64_t value);

int state_dump_reg(char * dest, int max, enum state_reg_t reg);

#endif /* __STATE_H */

