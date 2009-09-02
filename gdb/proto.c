/* proto.c
 * GDB remote serial protocol implementation
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

#define GDB_BUF_SIZE		1024

#include <minilib.h>
#include <demap.h>
#include <tables.h>

#include <lwip/tcp.h>

#include "stub.h"

#define GDB_PORT	2159

enum fsm_result { NEEDMORE, OK, FAIL };

static struct tcp_pcb * last_conn = NULL;

void enhexificate (void * ibuf, char * obuf, int len) {
	unsigned char * cibuf = ibuf;
	while (len > 0) {
		btohex(obuf, *(cibuf++));
		obuf += 2;
		len--;
	}

	*obuf = '\0';
}

void rle (char * buf) {
	char * inptr = buf;
	char * outptr = buf;
	int lastchar = buf[0];
	int repcount = 0;

	do {
		if (*inptr == lastchar && repcount < 97) {
			repcount += 1;
		} else {
			*(outptr++) = lastchar;

			if (repcount == 2)
				*(outptr++) = lastchar;

			while (repcount == 6 || repcount == 7) {
				*(outptr++) = lastchar;
				repcount--;
			}

			if (repcount > 2) {
				*(outptr++) = '*';
				*(outptr++) = repcount + 29;
			}

			repcount = 1;
			lastchar = *inptr;
		}

		inptr++;
	} while (lastchar);

	*(outptr) = 0;
}


struct gdb_state {
	char data[128];
	int writepos;
	int readpos;
};


static void close_conn(struct tcp_pcb *pcb, struct gdb_state *state) {
	outputf("close_conn: bailing");
	set_run_mode(RM_UNENCUMBERED);
	last_conn = NULL;
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	mem_free(state);
	tcp_close(pcb);
	outputf("close_conn: done");
}

static int dehexbyte (char * p) {
	int v0, v1;
	v0 = dehexit (p[0]);
	v1 = dehexit (p[1]);
	if (v0 < 0 || v1 < 0) return -1;
	return v0 << 4 | v1;
}

static void finish_and_send(struct tcp_pcb *pcb, char * output_buf, int datalength) {
	char * p;
	unsigned char checksum = 0;

	output_buf[0] = '+';
	output_buf[1] = '$';

	for (p = output_buf + 2; p < output_buf + datalength + 2; p++)
		checksum += *(unsigned char *)p;

	output_buf[datalength + 2] = '#';

	btohex(output_buf + datalength + 3, checksum);
	tcp_write(pcb, output_buf, datalength + 5, TCP_WRITE_FLAG_COPY);
}

void send_stop_packet() {
	if (!last_conn) return;
	tcp_write(last_conn, "$T05thread:01;#07", 17, 0);
}

static enum fsm_result recv_fsm(struct tcp_pcb *pcb, struct gdb_state *state) {
	char * endp;
	char * p;
	unsigned char checksum = 0;
	int i;
	uint64_t addr;
	uint64_t length;

	char output_buf[256];

	/* Make sure we have at least 4 bytes (the size of the smallest legal
	   packet), and that the packet does in fact start with $. */

	if ((state->data[0] == '+') || (state->data[0] == '-')) {
		outputf("GDB: chomp");
		state->readpos++;
		return OK;
	}

	if (state->writepos < 4) return NEEDMORE;

	if (state->data[0] != '$') return FAIL;

	/* Find the end; make sure there's room for the checksum after. */

	endp = memchr(state->data, '#', state->writepos);

	if ((!endp) || (endp - state->data > state->writepos - 3))
		return NEEDMORE;

	/* Checksum. */

	for (p = state->data + 1; p < endp; p++)
		checksum += *(unsigned char *)p;

	if (checksum != dehexbyte(endp + 1))
	{
		outputf("GDB: bad checksum: %d vs %d", checksum , dehexbyte(endp + 1));
		return FAIL;
	}

	/* Null-terminate, for processing convenience */
	*endp = '\0';

	outputf("GDB: Got \'%s\'", state->data + 1);

	/* OK, process the packet */

	switch (state->data[1]) {
	case '?':
		tcp_write(pcb, "+$T05thread:01;#07", 18, 0);
		break;
	case 'g':
		read_registers_32(output_buf + 2);
		finish_and_send(pcb, output_buf, 128);
		break;

	case 'm':
		/* Parse the address */
		p = memchr(state->data, ',', endp - state->data);
		if (!p) return FAIL;
		*p = '\0';
		addr = 0;
		length = 0;

		if (!dehexstring(&addr, state->data + 2, -1)) return FAIL;
		if (!dehexstring(&length, p + 1, -1)) return FAIL;

		outputf("GDB: read %d from %d", (uint32_t)length, (uint32_t)addr);

		if (length > 120) length = 120;

		for (i = 0; i < length; i++) {
			p = demap(addr++);
			if (!p) break;
			btohex(output_buf + 2 + (2*i), *(char *)p);
		}

		finish_and_send(pcb, output_buf, 2*i);
		break;

	case 's':
		/* Step. */
		set_run_mode(RM_STEPPING);
		tcp_write(pcb, "+", 1, 0);
		break;
		
	default:
		tcp_write(pcb, "+$#00", 5, 0);
		break;
	}

	state->readpos += (endp - state->data) + 3;

	return OK;
}

err_t
gdb_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	struct gdb_state *state = arg;
	uint16_t copylen;

	if (err != ERR_OK) {
		outputf("GDB: recv err %d", err);
		/* FIXME do something better here? */
		return ERR_OK;
	}

	if (p == NULL) {
		outputf("GDB: Connection closed");
		close_conn(pcb, state);
		return ERR_OK;
	}

	if (p->tot_len > (GDB_BUF_SIZE - state->writepos)) {
		/* Overflow! */
		outputf("GDB: Overflow!");
		close_conn(pcb, state);
		return ERR_OK;
	}

	copylen = pbuf_copy_partial(p, state->data + state->writepos, p->tot_len, 0);

	outputf("GDB: Processing %d, wp %d, cp %d", p->tot_len, state->writepos, copylen);

	state->writepos += p->tot_len;

	tcp_recved(pcb, p->tot_len);
	pbuf_free(p);

	while (1) {
		switch (recv_fsm(pcb, state)) {
		case NEEDMORE:
			outputf("GDB FSM: blocking");
			goto doneprocessing;

		case OK:
			if (state->readpos == state->writepos) {
				state->readpos = 0;
				state->writepos = 0;
				goto doneprocessing;
			} else {
				memmove(state->data,
					state->data + state->readpos,
					state->writepos - state->readpos);
				state->writepos -= state->readpos;
				state->readpos = 0;
			}
			break;
		case FAIL:
			/* Shit */
			outputf("GDB: Protocol error");
			close_conn(pcb, state);
			return ERR_OK;
		}
	}

doneprocessing:
	return ERR_OK;
}

static err_t gdb_sent(void *arg, struct tcp_pcb *pcb, uint16_t len) {
/*
	struct gdb_state *state = arg;
	send_fsm(pcb, state);
*/
	return ERR_OK;
}

static err_t gdb_poll(void *arg, struct tcp_pcb *pcb) {
	/* Nothing? */
	return ERR_OK;
}


static err_t gdb_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
	struct gdb_state *state;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	outputf("GDB: accept");

	last_conn = pcb;

	set_run_mode(RM_STOPPED);

	state = (struct gdb_state *)mem_malloc(sizeof(struct gdb_state));

	if (!state)
	{
		outputf("gdb_accept: out of memory\n");
		return ERR_MEM;
	}

	memset(state, 0, sizeof(struct gdb_state));

	tcp_arg(pcb, state);
	tcp_recv(pcb, gdb_recv);
	tcp_sent(pcb, gdb_sent);
	tcp_poll(pcb, gdb_poll, 1);
/*
	tcp_err(pcb, gdb_err);
*/
	return ERR_OK;
}


void gdb_init() {
	struct tcp_pcb *pcb;
	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, GDB_PORT);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, gdb_accept);
} 

PROTOCOL(gdb_init);
