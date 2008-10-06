#include <pcap.h>
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

void out(char *buf, unsigned short start)
{
  int y,x;
  buf += start;
  printf("\x1B[0;0H");
  for (y = 0; y < 25; y++)
  {
    for (x = 0; x < 80; x++)
    {
      unsigned char c, col;
      
      c = *(buf++);
      col = *(buf++);
      
      printf("\x1B[0;");
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
        printf("1;");
      if (col & 0x80)
        printf("5;");
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
      
      printf("m%c", c);
    }
    printf("\n");
  }
}

void handler (u_char * user, const struct pcap_pkthdr *h, const u_char *bytes)
{
	netwatch_frame * f = (netwatch_frame *) bytes;

	int data_len = h->caplen - sizeof(netwatch_frame);

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

	out(buf, f->buf_window);
}

int main() {
	pcap_t * pcap;

	pcap = pcap_open_live(NULL, 65535, 1, 0, errbuf);
 
	if (!pcap)
	{
		printf("pcap_open_live: %s\n", errbuf);
		exit(1);
	}

	pcap_loop(pcap, -1, handler, NULL);

	return 0;
}
