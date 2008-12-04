#include <stdint.h>
#include <minilib.h>
#include <output.h>
#include <fb.h>

#include "lwip/tcp.h"

#include "rfb.h"

#define SET_PIXEL_FORMAT	0
#define SET_ENCODINGS		2
#define FB_UPDATE_REQUEST	3
#define KEY_EVENT		4
#define POINTER_EVENT		5
#define CLIENT_CUT_TEXT		6

#define RFB_BUF_SIZE	64

struct pixel_format {
	uint8_t bpp;
	uint8_t depth;
	uint8_t big_endian;
	uint8_t true_color;
	uint16_t red_max;
	uint16_t green_max;
	uint16_t blue_max;
	uint8_t red_shift;
	uint8_t green_shift;
	uint8_t blue_shift;
	uint8_t padding[3];
};

struct server_init_message {
	uint16_t fb_width;
	uint16_t fb_height;
	struct pixel_format fmt;
	uint32_t name_length;
	char name_string[8];
};

struct fb_update_req {
	uint8_t msgtype;
	uint8_t incremental;
	uint16_t xpos;
	uint16_t ypos;
	uint16_t width;
	uint16_t height;
};

struct set_encs_req {
	uint8_t msgtype;
	uint8_t padding;
	uint16_t num;
	int32_t encodings[];
};

struct key_event_pkt {
	uint8_t msgtype;
	uint8_t downflag;
	uint8_t pad[2];
	uint32_t keysym;
};

struct pointer_event_pkt {
	uint8_t msgtype;
	uint8_t button_mask;
	uint16_t x;
	uint16_t y;
};

struct text_event_pkt {
	uint8_t msgtype;
	uint8_t padding[3];
	uint32_t length;
	char text[];
};

struct update_header {
	uint8_t msgtype;
	uint8_t padding;
	uint16_t nrects;
	uint16_t xpos;
	uint16_t ypos;
	uint16_t width;
	uint16_t height;
	int32_t enctype;
};

struct rfb_state {
	enum {
		ST_BEGIN,
		ST_CLIENTINIT,
		ST_MAIN
	} state;
	int version;
	int encs_remaining;

	char data[RFB_BUF_SIZE];
	int readpos;
	int writepos;

	char next_update_incremental;
	char update_requested;

	struct fb_update_req client_interest_area;

	enum {
		SST_IDLE,
		SST_NEEDS_UPDATE,
		SST_SENDING
	} send_state;

	uint32_t update_pos;
	uint32_t frame_bytes;
};

static struct server_init_message server_info;

static void init_server_info() {
	server_info.name_length = htonl(8);
	memcpy(server_info.name_string, "NetWatch", 8);
}

static void update_server_info() {
	if (fb != NULL) {
		outputf("RFB: setting fmt %d", fb->curmode.format);
		server_info.fb_width = htons(fb->curmode.xres);
		server_info.fb_height = htons(fb->curmode.yres);
		switch (fb->curmode.format) {
		case FB_RGB888:
			server_info.fmt.bpp = 32;
			server_info.fmt.depth = 24;
			server_info.fmt.big_endian = 0;
			server_info.fmt.true_color = 1;
			server_info.fmt.red_max = htons(255);
			server_info.fmt.green_max = htons(255);
			server_info.fmt.blue_max = htons(255);
			server_info.fmt.red_shift = 0;
			server_info.fmt.green_shift = 8;
			server_info.fmt.blue_shift = 16;
			break;
		default:
			outputf("RFB: unknown fb fmt %d", fb->curmode.format);
			break;
		}
	} else {
		outputf("RFB: fb null");
	}
}

static void send_fsm(struct tcp_pcb *pcb, struct rfb_state *state) {
	struct update_header hdr;
	int left, sndlength;
	err_t err;

	switch (state->send_state) {
	case SST_IDLE:
		/* Nothing to do */
		if (state->update_requested) {
			outputf("RFB send: update requested");
			state->update_requested = 0;
			state->send_state = SST_NEEDS_UPDATE;
		} else {
			break;
		}
	
		/* potential FALL THROUGH */

	case SST_NEEDS_UPDATE:
		outputf("RFB send: sending header");
		/* Send a header */
		state->frame_bytes = fb->curmode.xres * fb->curmode.yres * 3; /* XXX */
		hdr.msgtype = 0;
		hdr.nrects = htons(1);
		hdr.xpos = htons(0);
		hdr.ypos = htons(0);
		hdr.width = htons(fb->curmode.xres);
		hdr.height = htons(fb->curmode.yres);
		hdr.enctype = htonl(0);
		tcp_write(pcb, &hdr, sizeof(hdr), 0);
		tcp_output(pcb);

		state->update_pos = 0;
		state->send_state = SST_SENDING;

		/* FALL THROUGH */

	case SST_SENDING:
		left = state->frame_bytes - state->update_pos;

		if (left > tcp_sndbuf(pcb)) {
			sndlength = tcp_sndbuf(pcb);
		} else {
			sndlength = left;
		}

		do {
			err = tcp_write(pcb, fb->fbaddr + state->update_pos, sndlength, 0);
			if (err == ERR_MEM) {
				outputf("RFB: ERR_MEM sending %d", sndlength);
				sndlength /= 2;
			}
		} while (err == ERR_MEM && sndlength > 1);

		if (err == ERR_OK) {
			outputf("RFB: sent %d", sndlength);
			state->update_pos += sndlength;
		} else {
			outputf("RFB: send error %d", err);
		}

		tcp_output(pcb);

		if (state->update_pos == state->frame_bytes) {
			state->send_state = SST_IDLE;
		}

		break;
	}
}

static err_t rfb_sent(void *arg, struct tcp_pcb *pcb, uint16_t len) {
	struct rfb_state *state = arg;
	send_fsm(pcb, state);
	return ERR_OK;
}

static void close_conn(struct tcp_pcb *pcb, struct rfb_state *state) {
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	mem_free(state);
	tcp_close(pcb);
}

enum fsm_result {
	NEEDMORE,
	OK,
	FAIL
};

static enum fsm_result recv_fsm(struct tcp_pcb *pcb, struct rfb_state *state) {
	int i;
	int pktsize;

	outputf("RFB FSM: st %d rp %d wp %d", state->state, state->readpos,
		state->writepos);

	switch(state->state) {
	case ST_BEGIN:
		if (state->writepos < 12) return NEEDMORE;

		if (!strncmp(state->data, "RFB 003.003\n", 12)) {
			state->version = 3;
		} else if (!strncmp(state->data, "RFB 003.005\n", 12)) {
			/* Spec states that "RFB 003.005", an incorrect value,
			 * should be treated by the server as 3.3. */
			state->version = 3;
		} else if (!strncmp(state->data, "RFB 003.007\n", 12)) {
			state->version = 7;
		} else if (!strncmp(state->data, "RFB 003.008\n", 12)) {
			state->version = 8;
		} else {
			outputf("RFB: Negotiation fail");
			return FAIL;
		}

		outputf("RFB: Negotiated v3.%d", state->version);

		state->readpos += 12;
		state->state = ST_CLIENTINIT;

		/* We support one security type, currently "none".
		 * Send that and SecurityResult. */
		if (state->version >= 7) {
			tcp_write(pcb, "\x01\x01\x00\x00\x00\x00", 6, 0);
		} else {
			tcp_write(pcb, "\x01\x00\x00\x00\x00", 5, 0);
		}

		tcp_output(pcb);

		return OK;

	case ST_CLIENTINIT:
		if (state->version >= 7) {
			/* Ignore the security type and ClientInit */
			if (state->writepos < 2) return NEEDMORE;
			state->readpos += 2;
		} else {
			/* Just ClientInit */
			if (state->writepos < 1) return NEEDMORE;
			state->readpos += 1;
		}

		state->state = ST_MAIN;

		outputf("RFB: Sending server info", state->version);
		tcp_write(pcb, &server_info, sizeof(server_info), 0);
		tcp_output(pcb);

		return OK;

	case ST_MAIN:
		if (state->writepos < 1) return NEEDMORE;

		outputf("RFB: cmd %d", state->data[0]);
		switch (state->data[0]) {

		case SET_PIXEL_FORMAT:
			/* SetPixelFormat */
			if (state->writepos < (sizeof(struct pixel_format) + 4))
				return NEEDMORE;
			outputf("RFB: SetPixelFormat");
/*
			struct pixel_format * new_fmt =
				(struct pixel_format *)(&state->data[4]);
*/
			/* XXX ... */

			state->readpos += sizeof(struct pixel_format) + 4;
			return OK;

		case SET_ENCODINGS:
			if (state->writepos < 4) return NEEDMORE;

			struct set_encs_req * req = (struct set_encs_req *)state->data;

			pktsize = sizeof(struct set_encs_req) + (4 * ntohs(req->num));

			outputf("RFB: SetEncodings [%d]", ntohs(req->num));
			if (state->writepos < pktsize) return NEEDMORE;

			for (i = 0; i < ntohs(req->num); i++) {
				outputf("RFB: Encoding: %d", ntohl(req->encodings[i]));
				/* XXX ... */

			}

			state->readpos += pktsize;
			return OK;

		case FB_UPDATE_REQUEST:
			if (state->writepos < sizeof(struct fb_update_req))
				return NEEDMORE;
			outputf("RFB: UpdateRequest");

			state->update_requested = 1;
			memcpy(&state->client_interest_area, state->data,
			       sizeof(struct fb_update_req)); 

			state->readpos += sizeof(struct fb_update_req);
			return OK;

		case KEY_EVENT:
			if (state->writepos < sizeof(struct key_event_pkt))
				return NEEDMORE;
			outputf("RFB: Key");

			/* XXX stub */

			state->readpos += sizeof(struct key_event_pkt);
			return OK;

		case POINTER_EVENT:
			if (state->writepos < sizeof(struct pointer_event_pkt))
				return NEEDMORE;
			outputf("RFB: Pointer");

			/* XXX stub */

			state->readpos += sizeof(struct pointer_event_pkt);
			return OK;

		case CLIENT_CUT_TEXT:
			if (state->writepos < sizeof(struct text_event_pkt))
				return NEEDMORE;
			outputf("RFB: Cut Text");

			struct text_event_pkt * pkt =
				(struct text_event_pkt *)state->data;

			if (state->writepos < sizeof(struct text_event_pkt)
			                      + pkt->length)
				return NEEDMORE;

			/* XXX stub */

			state->readpos += sizeof(struct text_event_pkt)
			                  + pkt->length;
			return OK;

		default:
			outputf("RFB: Bad command: %d", state->data[0]);
		}
	default:
		outputf("RFB: Bad state");
		return FAIL;
	}
}

static err_t rfb_recv(void *arg, struct tcp_pcb *pcb,
                      struct pbuf *p, err_t err) {
	struct rfb_state *state = arg;

	if (state == NULL) 

	if (err != ERR_OK) {
		outputf("RFB: recv err %d", err);
		/* FIXME do something better here? */
		return ERR_OK;
	}

	if (p == NULL) {
		outputf("RFB: Connection closed");
		close_conn(pcb, state);
		return ERR_OK;
	}

	if (p->tot_len > (RFB_BUF_SIZE - state->writepos)) {
		/* Overflow! */
		outputf("RFB: Overflow!");
		close_conn(pcb, state);
		return ERR_OK;
	}

	outputf("RFB: Processing %d", p->tot_len);
	pbuf_copy_partial(p, state->data + state->writepos, p->tot_len, 0);
	state->writepos += p->tot_len;

	tcp_recved(pcb, p->tot_len);
	pbuf_free(p);

	while (1) {
		switch (recv_fsm(pcb, state)) {
		case NEEDMORE:
			outputf("RFB FSM: blocking");
			/* Need more data */
			return ERR_OK;

		case OK:
			outputf("RFB FSM: ok");

			/* Might as well send now... */
			if (state->send_state == SST_IDLE
			    && state->update_requested) {
				send_fsm(pcb, state);
			}

			if (state->readpos == state->writepos) {
				state->readpos = 0;
				state->writepos = 0;
				return ERR_OK;
			} else {
				memmove(state->data,
				        state->data + state->readpos,
				        state->writepos - state->readpos);
			}
			break;
		case FAIL:
			/* Shit */
			outputf("RFB: Protocol error");
			close_conn(pcb, state);
			return ERR_OK;
		}
	}
}	
		
static err_t rfb_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
	struct rfb_state *state;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	state = (struct rfb_state *)mem_malloc(sizeof(struct rfb_state));

	state->state = ST_BEGIN;
	state->readpos = 0;
	state->writepos = 0;
	state->update_requested = 0;
	state->send_state = SST_IDLE;

	/* XXX: update_server_info() should be called from the 64ms timer, and deal
	 * with screen resizes appropriately. */
	update_server_info();

	if (!state)
	{
		outputf("rfb_accept: out of memory\n");
		return ERR_MEM;
	}

	tcp_arg(pcb, state);
	tcp_recv(pcb, rfb_recv);
	tcp_sent(pcb, rfb_sent);
/*
	tcp_err(pcb, rfb_err);
	tcp_poll(pcb, rfb_poll, 2);
*/
	tcp_write(pcb, "RFB 003.008\n", 12, 0);
	tcp_output(pcb);

	return ERR_OK;
}

void rfb_init() {
	struct tcp_pcb *pcb;

	init_server_info();

	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, RFB_PORT);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, rfb_accept);
}
