#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#define NO_SYS 1


#define LWIP_SOCKET	0
#define LWIP_NETCONN	0

#define LWIP_SNMP	0

#define LWIP_DHCP	1

/* For big ones... */
extern void _memcpy(void *dest, const void *src, int bytes);
#define MEMCPY(dst,src,len) _memcpy(dst,src,len)

/* Lots of tricks from http://lists.gnu.org/archive/html/lwip-users/2006-11/msg00007.html */

#define MEM_SIZE	65536
#define TCP_MSS         1460
#define TCP_WND		24000
#define TCP_SND_BUF     (16 * TCP_MSS)
#define TCP_SND_QUEUELEN 16

#define MEMP_NUM_PBUF	128

#endif
