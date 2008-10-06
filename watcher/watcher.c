#include <pcap.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

char errbuf[PCAP_ERRBUF_SIZE];

char buf[0x8000];

typedef struct netwatch_frame {
	char dest_mac[6];
	char src_mac[6];
	unsigned short whatthefuck;
	unsigned short ethertype;
	unsigned short buf_offset;
	unsigned short buf_window;
	char data[];
} netwatch_frame;

pcap_t * pcap;

void out(char *buf, unsigned short start)
{
  int y,x;
  buf += start;
  printf("\x1B[H");
  for (y = 0; y < 25; y++)
  {
    printf("\x1B[%d;%df",y+1,1);
    for (x = 0; x < 80; x++)
    {
      unsigned char c, col;
      static unsigned char last = 0xFF;
      
      c = *(buf++);
      col = *(buf++);

      if (col != last)
      {
      printf("\x1B[0m\x1B[");
      switch (col & 0x7)
      {
      case 0: printf("30;"); break;
      case 1: printf("34;"); break;
      case 2: printf("32;"); break;
      case 3: printf("36;"); break;
      case 4: printf("31;"); break;
      case 5: printf("35;"); break;
      case 6: printf("33;"); break;
      case 7: printf("37;"); break;
      }
      if (col & 0x8)
        printf("m\x1B[1;");
      if (col & 0x80)
        printf("m\x1B[5;");
      switch ((col >> 4) & 0x7)
      {
      case 0: printf("40"); break;
      case 1: printf("44"); break;
      case 2: printf("42"); break;
      case 3: printf("46"); break;
      case 4: printf("41"); break;
      case 5: printf("45"); break;
      case 6: printf("43"); break;
      case 7: printf("47"); break;
      }
      printf("m");
      }
      last = col;
      if (c == 0)
      	c = ' ';
      if (!isprint(c))
        c = 'X';
      printf("%c", c);
      fflush(stdout);
    }
  }
}




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

typedef struct frame {
	char header[14];
	unsigned short datalen;
	unsigned char cmd;
	char data[];
} frame;

int sendsock;

void send_text(char *d, int len)
{
	int datalen = sizeof(frame) + len;
	frame * f = alloca(datalen + 1);
	struct sockaddr_ll addr;

	memcpy(f->header, "\x00\xb0\xd0\x97\xbc\xac\x00\x03\x93\x87\x84\x8C\x13\x38", 14);
	f->datalen = len;
	f->cmd = 0x42;
	memcpy(f->data, d, len);
	
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = 2;
	addr.sll_halen = 0;

 	if (sendto(sendsock, f, datalen, 0, (struct sockaddr *)&addr, sizeof(addr)) < datalen)
		perror("sendto");
}

void handler (u_char * user, const struct pcap_pkthdr *h, const u_char *bytes)
{
	netwatch_frame * f = (netwatch_frame *) bytes;
	static int i = 0;

	int data_len = h->caplen - sizeof(netwatch_frame) - 4;

	if (data_len < 0) return;

	if (ntohs(f->ethertype) != 0x1337) return;

	if (data_len > sizeof(buf)) return;

	if (f->buf_offset + data_len > sizeof(buf))
	{
		int wrap_pos = sizeof(buf) - f->buf_offset;
		memcpy(buf + f->buf_offset, f->data, wrap_pos);
		memcpy(buf, f->data + wrap_pos, data_len - wrap_pos);
	} else {
		memcpy(buf + f->buf_offset, f->data, data_len);
	}

	int c = getchar();
	if (c > 0) {
		if (c == 3)
			pcap_breakloop(pcap);
		if (c == '\r') c = '\n';
		if (c == 127) c = '\b';

		char ch = c;
		send_text(&ch, 1);
	}

	if (!(i++ % 2))
		out(buf, f->buf_window);
}

int main() {
	pcap = pcap_open_live(NULL, 65535, 1, 0, errbuf);
 
	if (!pcap)
	{
		printf("pcap_open_live: %s\n", errbuf);
		exit(1);
	}
	sendsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sendsock < 0)
	{
		perror("socket");
		exit(1);
	}
	fcntl(0, F_SETFL, O_NONBLOCK);
	printf("\x1b[H\x1b[J\x1b[5C");

	struct termios t, t_orig;
	tcgetattr(0, &t);
	tcgetattr(0, &t_orig);
	cfmakeraw(&t);
	tcsetattr(0, 0, &t);

	pcap_loop(pcap, -1, handler, NULL);

	tcsetattr(0, 0, &t_orig);
	return 0;
}
