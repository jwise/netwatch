/* reg-x86.h
 * X86 standard registers
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

#ifndef __REG_X86_H
#define __REG_X86_H

#define CR0_PE		(1<<0)
#define CR0_MP		(1<<1)
#define CR0_EM		(1<<2)
#define CR0_TS		(1<<3)
#define CR0_ET		(1<<4)
#define CR0_NE		(1<<5)
#define CR0_WP		(1<<16)
#define CR0_AM		(1<<18)
#define CR0_NW		(1<<29)
#define CR0_CD		(1<<30)
#define CR0_PG		(1<<31)

#define CR3_PWT		(1<<3)
#define CR3_PCD		(1<<4)

#define CR4_VME		(1<<0)
#define CR4_PVI		(1<<1)
#define CR4_TSD		(1<<2)
#define CR4_DE		(1<<3)
#define CR4_PSE		(1<<4)
#define CR4_PAE		(1<<5)
#define CR4_MCE		(1<<6)
#define CR4_PGE		(1<<7)
#define CR4_PCE		(1<<8)
#define CR4_OSFXSR	(1<<9)
#define CR4_OSXMMEXCPT	(1<<10)

#define EFLAGS_CF	(1<<0)
#define EFLAGS_PF	(1<<2)
#define EFLAGS_AF	(1<<4)
#define EFLAGS_ZF	(1<<6)
#define EFLAGS_SF	(1<<7)
#define EFLAGS_TF	(1<<8)
#define EFLAGS_IF	(1<<9)
#define EFLAGS_DF	(1<<10)
#define EFLAGS_OF	(1<<11)
#define EFLAGS_IOPL	(3<<12)
#define EFLAGS_NT	(1<<14)
#define EFLAGS_RF	(1<<16)
#define EFLAGS_VM	(1<<17)
#define EFLAGS_AC	(1<<18)
#define EFLAGS_VIF	(1<<19)
#define EFLAGS_VIP	(1<<20)
#define EFLAGS_ID	(1<<21)

#endif /* __REG_X86_H */

