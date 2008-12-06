#include <pci.h>
#include <smram.h>
#include <pci-bother.h>
#include <output.h>
#include <minilib.h>
#include <lwip/init.h>
#include "net.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include <lwip/dhcp.h>
#include <lwip/tcp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include "rfb.h"

static struct nic *_nic = 0x0;
static struct netif _netif;

extern struct pci_driver a3c90x_driver;

void eth_recv(struct nic *nic, struct pbuf *p)
{
	struct eth_hdr *ethhdr;
	
	LINK_STATS_INC(link.recv);
	
	ethhdr = p->payload;
	
	switch (htons(ethhdr->type)) {
	case ETHTYPE_IP:
	case ETHTYPE_ARP:
		if (_netif.input(p, &_netif) != ERR_OK)
		{
			LWIP_DEBUGF(NETIF_DEBUG, ("netdev_input: IP input error\n"));
			pbuf_free(p);
		}
		break;
		
	default:
		outputf("Unhandled packet type %04x input", ethhdr->type);
		pbuf_free(p);
		break;
	}
}

void eth_poll()
{
	static int ticks = 0;
	int i = 15;	/* Don't process more than 15 packets at a time; we don't want the host to get TOO badly slowed down... */
	
	if (!_nic)
		return;
	
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	
	if ((ticks % 1000) == 0)	/* About a minute */
		dhcp_coarse_tmr();
	if ((ticks % 8) == 0)	/* About 500msec*/
		dhcp_fine_tmr();
	if ((ticks % 4) == 0)	/* About 250msec*/
		tcp_tmr();
	ticks++;

	while (i > 0)
	{
		int n = _nic->recv(_nic);
		i -= n;
		if (n == 0)
			break;
	}
}

static err_t _transmit(struct netif *netif, struct pbuf *p)
{
	struct nic *nic = netif->state;

//	outputf("NIC: Transmit packet");

	nic->transmit(p);

	LINK_STATS_INC(link.xmit);

	return ERR_OK;
}

static err_t _init(struct netif *netif)
{
	struct nic *nic = netif->state;
	
	LWIP_ASSERT("netif != NULL", (netif != NULL));
		
#if LWIP_NETIF_HOSTNAME
	netif->hostname = "netwatch";
#endif
	
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

	netif->name[0] = 'e';
	netif->name[1] = 'n';
	netif->output = etharp_output;
	netif->linkoutput = _transmit;
	
	memcpy(netif->hwaddr, nic->hwaddr, 6);
	netif->mtu = 1500;
	netif->hwaddr_len = ETHARP_HWADDR_LEN;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	
	return ERR_OK;
}

int eth_register(struct nic *nic)
{
	static struct ip_addr ipa = { 0 } , netmask = { 0 } , gw = { 0 };
	
	if (_nic)
		return -1;
	netif_add(&_netif, &ipa, &netmask, &gw, (void*)nic, _init, ethernet_input);
	netif_set_default(&_netif);
	netif_set_up(&_netif);
	dhcp_start(&_netif);
	_nic = nic;
	return 0;
}

void eth_init()
{
	extern void httpd_init();
	
	/* Required for DMA to work. :( */
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	lwip_init();
	httpd_init();

	rfb_init();

}
