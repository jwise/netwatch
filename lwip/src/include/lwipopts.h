#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#define NO_SYS 1


#define LWIP_SOCKET	0
#define LWIP_NETCONN	0

#define LWIP_SNMP	0

#define LWIP_DHCP	1

#define MEM_SIZE	65536
#define TCP_MSS         1400
#define TCP_WND		24000
#define TCP_SND_BUF     (16 * TCP_MSS)

#define MEMP_NUM_PBUF	128

#endif
