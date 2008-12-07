/*
 * 3c90x.c
 * NetWatch
 *
 * A ring buffer-based, bus-mastering Ethernet driver.
 *
 * Derived from Etherboot's 3c90x.c, which is
 *   Copyright (C) 1999 LightSys Technology Services, Inc.
 *   Portions Copyright (C) 1999 Steve Smith
 *
 * This program may be re-distributed in source or binary form, modified,
 * sold, or copied for any purpose, provided that the above copyright message
 * and this text are included with all source copies or derivative works, and
 * provided that the above copyright message and this text are included in the
 * documentation of any binary-only distributions.  This program is distributed
 * WITHOUT ANY WARRANTY, without even the warranty of FITNESS FOR A PARTICULAR
 * PURPOSE or MERCHANTABILITY.  Please read the associated documentation
 * "3c90x.txt" before compiling and using this driver.
 *
 * REVISION HISTORY:
 *
 * v0.10	1-26-1998	GRB	Initial implementation.
 * v0.90	1-27-1998	GRB	System works.
 * v1.00pre1	2-11-1998	GRB	Got prom boot issue fixed.
 * v2.0		9-24-1999	SCS	Modified for 3c905 (from 3c905b code)
 *					Re-wrote poll and transmit for
 *					better error recovery and heavy
 *					network traffic operation
 * v2.01        5-26-2003       NN      Fixed driver alignment issue which
 *                                      caused system lockups if driver structures
 *                                      not 8-byte aligned.
 * NetWatch0    12-07-2008      JAW     Taken out back and shot.
 *
 */

#include "etherboot-compat.h"
#include "net.h"
#include <timer.h>
#include <io.h>
#include <pci.h>
#include <pci-bother.h>
#include <minilib.h>
#include <output.h>
#include <paging.h>

#define	XCVR_MAGIC	(0x5A00)

/*** Register definitions for the 3c905 ***/
enum Registers
    {
    regPowerMgmtCtrl_w = 0x7c,        /** 905B Revision Only                 **/
    regUpMaxBurst_w = 0x7a,           /** 905B Revision Only                 **/
    regDnMaxBurst_w = 0x78,           /** 905B Revision Only                 **/
    regDebugControl_w = 0x74,         /** 905B Revision Only                 **/
    regDebugData_l = 0x70,            /** 905B Revision Only                 **/
    regRealTimeCnt_l = 0x40,          /** Universal                          **/
    regUpBurstThresh_b = 0x3e,        /** 905B Revision Only                 **/
    regUpPoll_b = 0x3d,               /** 905B Revision Only                 **/
    regUpPriorityThresh_b = 0x3c,     /** 905B Revision Only                 **/
    regUpListPtr_l = 0x38,            /** Universal                          **/
    regCountdown_w = 0x36,            /** Universal                          **/
    regFreeTimer_w = 0x34,            /** Universal                          **/
    regUpPktStatus_l = 0x30,          /** Universal with Exception, pg 130   **/
    regTxFreeThresh_b = 0x2f,         /** 90X Revision Only                  **/
    regDnPoll_b = 0x2d,               /** 905B Revision Only                 **/
    regDnPriorityThresh_b = 0x2c,     /** 905B Revision Only                 **/
    regDnBurstThresh_b = 0x2a,        /** 905B Revision Only                 **/
    regDnListPtr_l = 0x24,            /** Universal with Exception, pg 107   **/
    regDmaCtrl_l = 0x20,              /** Universal with Exception, pg 106   **/
                                      /**                                    **/
    regIntStatusAuto_w = 0x1e,        /** 905B Revision Only                 **/
    regTxStatus_b = 0x1b,             /** Universal with Exception, pg 113   **/
    regTimer_b = 0x1a,                /** Universal                          **/
    regTxPktId_b = 0x18,              /** 905B Revision Only                 **/
    regCommandIntStatus_w = 0x0e,     /** Universal (Command Variations)     **/
    };

/** following are windowed registers **/
enum Registers7
    {
    regPowerMgmtEvent_7_w = 0x0c,     /** 905B Revision Only                 **/
    regVlanEtherType_7_w = 0x04,      /** 905B Revision Only                 **/
    regVlanMask_7_w = 0x00,           /** 905B Revision Only                 **/
    };

enum Registers6
    {
    regBytesXmittedOk_6_w = 0x0c,     /** Universal                          **/
    regBytesRcvdOk_6_w = 0x0a,        /** Universal                          **/
    regUpperFramesOk_6_b = 0x09,      /** Universal                          **/
    regFramesDeferred_6_b = 0x08,     /** Universal                          **/
    regFramesRecdOk_6_b = 0x07,       /** Universal with Exceptions, pg 142  **/
    regFramesXmittedOk_6_b = 0x06,    /** Universal                          **/
    regRxOverruns_6_b = 0x05,         /** Universal                          **/
    regLateCollisions_6_b = 0x04,     /** Universal                          **/
    regSingleCollisions_6_b = 0x03,   /** Universal                          **/
    regMultipleCollisions_6_b = 0x02, /** Universal                          **/
    regSqeErrors_6_b = 0x01,          /** Universal                          **/
    regCarrierLost_6_b = 0x00,        /** Universal                          **/
    };

enum Registers5
    {
    regIndicationEnable_5_w = 0x0c,   /** Universal                          **/
    regInterruptEnable_5_w = 0x0a,    /** Universal                          **/
    regTxReclaimThresh_5_b = 0x09,    /** 905B Revision Only                 **/
    regRxFilter_5_b = 0x08,           /** Universal                          **/
    regRxEarlyThresh_5_w = 0x06,      /** Universal                          **/
    regTxStartThresh_5_w = 0x00,      /** Universal                          **/
    };

enum Registers4
    {
    regUpperBytesOk_4_b = 0x0d,       /** Universal                          **/
    regBadSSD_4_b = 0x0c,             /** Universal                          **/
    regMediaStatus_4_w = 0x0a,        /** Universal with Exceptions, pg 201  **/
    regPhysicalMgmt_4_w = 0x08,       /** Universal                          **/
    regNetworkDiagnostic_4_w = 0x06,  /** Universal with Exceptions, pg 203  **/
    regFifoDiagnostic_4_w = 0x04,     /** Universal with Exceptions, pg 196  **/
    regVcoDiagnostic_4_w = 0x02,      /** Undocumented?                      **/
    };

enum Registers3
    {
    regTxFree_3_w = 0x0c,             /** Universal                          **/
    regRxFree_3_w = 0x0a,             /** Universal with Exceptions, pg 125  **/
    regResetMediaOptions_3_w = 0x08,  /** Media Options on B Revision,       **/
                                      /** Reset Options on Non-B Revision    **/
    regMacControl_3_w = 0x06,         /** Universal with Exceptions, pg 199  **/
    regMaxPktSize_3_w = 0x04,         /** 905B Revision Only                 **/
    regInternalConfig_3_l = 0x00,     /** Universal, different bit           **/
                                      /** definitions, pg 59                 **/
    };

enum Registers2
    {
    regResetOptions_2_w = 0x0c,       /** 905B Revision Only                 **/
    regStationMask_2_3w = 0x06,       /** Universal with Exceptions, pg 127  **/
    regStationAddress_2_3w = 0x00,    /** Universal with Exceptions, pg 127  **/
    };

enum Registers1
    {
    regRxStatus_1_w = 0x0a,           /** 90X Revision Only, Pg 126          **/
    };

enum Registers0
    {
    regEepromData_0_w = 0x0c,         /** Universal                          **/
    regEepromCommand_0_w = 0x0a,      /** Universal                          **/
    regBiosRomData_0_b = 0x08,        /** 905B Revision Only                 **/
    regBiosRomAddr_0_l = 0x04,        /** 905B Revision Only                 **/
    };


/*** The names for the eight register windows ***/
enum Windows
    {
    winPowerVlan7 = 0x07,
    winStatistics6 = 0x06,
    winTxRxControl5 = 0x05,
    winDiagnostics4 = 0x04,
    winTxRxOptions3 = 0x03,
    winAddressing2 = 0x02,
    winUnused1 = 0x01,
    winEepromBios0 = 0x00,
    };


/*** Command definitions for the 3c90X ***/
enum Commands
    {
    cmdGlobalReset = 0x00,             /** Universal with Exceptions, pg 151 **/
    cmdSelectRegisterWindow = 0x01,    /** Universal                         **/
    cmdEnableDcConverter = 0x02,       /**                                   **/
    cmdRxDisable = 0x03,               /**                                   **/
    cmdRxEnable = 0x04,                /** Universal                         **/
    cmdRxReset = 0x05,                 /** Universal                         **/
    cmdStallCtl = 0x06,                /** Universal                         **/
    cmdTxEnable = 0x09,                /** Universal                         **/
    cmdTxDisable = 0x0A,               /**                                   **/
    cmdTxReset = 0x0B,                 /** Universal                         **/
    cmdRequestInterrupt = 0x0C,        /**                                   **/
    cmdAcknowledgeInterrupt = 0x0D,    /** Universal                         **/
    cmdSetInterruptEnable = 0x0E,      /** Universal                         **/
    cmdSetIndicationEnable = 0x0F,     /** Universal                         **/
    cmdSetRxFilter = 0x10,             /** Universal                         **/
    cmdSetRxEarlyThresh = 0x11,        /**                                   **/
    cmdSetTxStartThresh = 0x13,        /**                                   **/
    cmdStatisticsEnable = 0x15,        /**                                   **/
    cmdStatisticsDisable = 0x16,       /**                                   **/
    cmdDisableDcConverter = 0x17,      /**                                   **/
    cmdSetTxReclaimThresh = 0x18,      /**                                   **/
    cmdSetHashFilterBit = 0x19,        /**                                   **/
    };


/*** Values for int status register bitmask **/
#define	INT_INTERRUPTLATCH	(1<<0)
#define INT_HOSTERROR		(1<<1)
#define INT_TXCOMPLETE		(1<<2)
#define INT_RXCOMPLETE		(1<<4)
#define INT_RXEARLY		(1<<5)
#define INT_INTREQUESTED	(1<<6)
#define INT_UPDATESTATS		(1<<7)
#define INT_LINKEVENT		(1<<8)
#define INT_DNCOMPLETE		(1<<9)
#define INT_UPCOMPLETE		(1<<10)
#define INT_CMDINPROGRESS	(1<<12)
#define INT_WINDOWNUMBER	(7<<13)

/* These structures are all 64-bit aligned, as needed for bus-mastering I/O. */
typedef struct {
	unsigned int addr;
	unsigned int len;
} segment_t __attribute__ ((aligned(8)));

typedef struct {
	unsigned int next;
	unsigned int hdr;
	segment_t segments[64 /* XXX magic */];
} txdesc_t __attribute__ ((aligned(8)));

/*** RX descriptor ***/
typedef struct {
	unsigned int	next;
	unsigned int	status;
	segment_t segments[64];
} rxdesc_t __attribute__ ((aligned(8)));

typedef struct {
	struct nic nic;
	int is3c556;
	int isBrev;
	int curwnd;
	int ioaddr;
} nic_3c90x_t;

static nic_3c90x_t _nic;

#define _inb(n,a) (inb((n)->ioaddr + (a)))
#define _inw(n,a) (inw((n)->ioaddr + (a)))
#define _inl(n,a) (inl((n)->ioaddr + (a)))

#define _outb(n,a,d) (outb((n)->ioaddr + (a), (d)))
#define _outw(n,a,d) (outw((n)->ioaddr + (a), (d)))
#define _outl(n,a,d) (outl((n)->ioaddr + (a), (d)))


static int _issue_command(nic_3c90x_t *nic, int cmd, int param)
{
	_outw(nic, regCommandIntStatus_w, (cmd << 11) | param);

	while (_inw(nic, regCommandIntStatus_w) & INT_CMDINPROGRESS)
		;

	return 0;
}


/*** a3c90x_internal_SetWindow: selects a register window set.
 ***/
static int _set_window(nic_3c90x_t *nic, int window)
{
	if (nic->curwnd == window)
		return 0;

	_issue_command(nic, cmdSelectRegisterWindow, window);
	nic->curwnd = window;

	return 0;
}


/*** _read_eeprom - read data from the serial eeprom.
 ***/
static unsigned short
_read_eeprom(nic_3c90x_t *nic, int address)
{
	unsigned short val;

	/** Select correct window **/
        _set_window(nic, winEepromBios0);

	/** Make sure the eeprom isn't busy **/
	do
        {
        	int i;
        	for (i = 0; i < 165; i++)
        		inb(0x80);	/* wait 165 usec */
        }
	while(0x8000 & _inw(nic, regEepromCommand_0_w));

	/** Read the value. **/
	if (nic->is3c556)
		_outw(nic, regEepromCommand_0_w, address + 0x230);
	else
		_outw(nic, regEepromCommand_0_w, address + 0x80);

	do
	{
		int i;
		for (i = 0; i < 165; i++)
			inb(0x80);	/* wait 165 usec */
	}
	while(0x8000 & _inw(nic, regEepromCommand_0_w));
	val = _inw(nic, regEepromData_0_w);
	
	return val;
}


#if 0
/*** a3c90x_reset: exported function that resets the card to its default
 *** state.  This is so the Linux driver can re-set the card up the way
 *** it wants to.  If CFG_3C90X_PRESERVE_XCVR is defined, then the reset will
 *** not alter the selected transceiver that we used to download the boot
 *** image.
 ***/
static void _reset(nic_3c90x_t *nic)
{
    /** Send the reset command to the card **/
    outputf("3c90x: issuing RESET");
    _issue_command(nic, cmdGlobalReset, 0);

    /** global reset command resets station mask, non-B revision cards
     ** require explicit reset of values
     **/
    _set_window(nic, winAddressing2);
    _outw(nic, regStationMask_2_3w+0, 0);
    _outw(nic, regStationMask_2_3w+2, 0);
    _outw(nic, regStationMask_2_3w+4, 0);

    /** Issue transmit reset, wait for command completion **/
    _issue_command(nic, cmdTxReset, 0);
    if (!nic->isBrev)
	_outb(nic, regTxFreeThresh_b, 0x01);
    _issue_command(nic, cmdTxEnable, 0);

    /**
     ** reset of the receiver on B-revision cards re-negotiates the link
     ** takes several seconds (a computer eternity)
     **/
    if (nic->isBrev)
	_issue_command(nic, cmdRxReset, 0x04);
    else
	_issue_command(nic, cmdRxReset, 0x00);
    _issue_command(nic, cmdRxEnable, 0);

    _issue_command(nic, cmdSetInterruptEnable, 0);
    /** enable rxComplete and txComplete **/
    _issue_command(nic, cmdSetIndicationEnable, 0x0014);
    /** acknowledge any pending status flags **/
    _issue_command(nic, cmdAcknowledgeInterrupt, 0x661);

    return;
}
#endif

/***************************** Transmit routines *****************************/

#define XMIT_BUFS 8

static txdesc_t txdescs[XMIT_BUFS];
static struct pbuf *txpbufs[XMIT_BUFS] = {0,};

/* txcons is the index into the ring buffer of the last packet that the
 * 3c90x was seen processing, or -1 if the 3c90x was idle.
 */
static int txcons = -1;

/* txprod is the index of the _next_ buffer that the driver will write into. */
static int txprod = 0;

/* _transmit adds a packet to the transmit ring buffer.  If no space is
 * available in the buffer, then _transmit blocks until a packet has been
 * transmitted.
 */
static void _transmit(struct nic *_nic, struct pbuf *p)
{
	nic_3c90x_t *nic = (nic_3c90x_t *)_nic;
	unsigned char status;
	int len, n;
	
	/* Wait for there to be space. */
	if (txcons == txprod)
	{
		int i = 0;
		
		outputf("3c90x: txbuf full, waiting for space...");
		while (_inl(nic, regDnListPtr_l) != 0)
			i++;
		outputf("3c90x: took %d iters", i);
	}
	
	/* Stall the download engine so it doesn't bother us. */
	_issue_command(nic, cmdStallCtl, 2 /* Stall download */);
	
	/* Clean up old txcons. */
	if (txcons != -1)
	{
		unsigned long curp = _inl(nic, regDnListPtr_l);
		int end;
		
		if (curp == 0)
			end = txprod;
		else
			end = (curp - v2p(txdescs)) / sizeof(txdescs[0]);
		
		while (txcons != end)
		{
			pbuf_free(txpbufs[txcons]);
			txpbufs[txcons] = NULL;
			txdescs[txcons].hdr = 0;
			txdescs[txcons].next = 0;
			txcons = (txcons + 1) % XMIT_BUFS;
		}
		if (txcons == txprod)
			txcons = -1;
	}
	
	/* Look at the TX status */
	status = _inb(nic, regTxStatus_b);
	if (status)
	{
		outputf("3c90x: error: the nus.");
		_outb(nic, regTxStatus_b, 0x00);
	}

	/* Set up the new txdesc. */
	txdescs[txprod].next = 0;
	len = 0;
	n = 0;
	txpbufs[txprod] = p;
	for (; p; p = p->next)
	{
		txdescs[txprod].segments[n].addr = v2p(p->payload);
		txdescs[txprod].segments[n].len = p->len | (p->next ? 0 : (1 << 31));
		len += p->len;
		pbuf_ref(p);
		n++;
	}
	txdescs[txprod].hdr = len;	/* If we wanted completion notification, bit 15 */
	
	/* Now link the new one in, after it's been set up. */
	txdescs[(txprod + XMIT_BUFS - 1) % XMIT_BUFS].next = v2p(&(txdescs[txprod]));
	
	/* If the card is stopped, start it up again. */
	if (_inl(nic, regDnListPtr_l) == 0)
	{
		_outl(nic, regDnListPtr_l, v2p(&(txdescs[txprod])));
		txcons = txprod;
	}
	
	txprod = (txprod + 1) % XMIT_BUFS;
	
	/* And let it proceed on its way. */
	_issue_command(nic, cmdStallCtl, 3 /* Unstall download */);

#if 0		
	/** successful completion (sans "interrupt Requested" bit) **/
	if ((status & 0xbf) == 0x80)
		return;

	outputf("3c90x: Status (%hhX)", status);
	/** check error codes **/
	if (status & 0x02)
	{
		outputf("3c90x: Tx Reclaim Error (%hhX)", status);
		_reset(nic);
	} else if (status & 0x04) {
		outputf("3c90x: Tx Status Overflow (%hhX)", status);
		for (i=0; i<32; i++)
			_outb(nic, regTxStatus_b, 0x00);
		/** must re-enable after max collisions before re-issuing tx **/
		_issue_command(nic, cmdTxEnable, 0);
	} else if (status & 0x08) {
		outputf("3c90x: Tx Max Collisions (%hhX)", status);
		/** must re-enable after max collisions before re-issuing tx **/
		_issue_command(nic, cmdTxEnable, 0);
	} else if (status & 0x10) {
		outputf("3c90x: Tx Underrun (%hhX)", status);
		_reset(nic);
	} else if (status & 0x20) {
		outputf("3c90x: Tx Jabber (%hhX)", status);
		_reset(nic);
	} else if ((status & 0x80) != 0x80) {
		outputf("3c90x: Internal Error - Incomplete Transmission (%hhX)", status);
		_reset(nic);
	}
#endif
}

/***************************** Receive routines *****************************/
#define MAX_RECV_SIZE 1536
#define RECV_BUFS 32

static rxdesc_t rxdescs[RECV_BUFS];
static struct pbuf *rxpbufs[RECV_BUFS] = {0,};

/* rxcons is the pointer to the receive descriptor that the ethernet card will
 * write into next.
 */
static int rxcons = 0;

/* rxprod is the pointer to the receive descriptor that the driver will
 * allocate next.
 */
static int rxprod = 0;

/* _recv_prepare fills the 3c90x's ring buffer with fresh pbufs from lwIP.
 * The upload engine need not be stalled.
 */
static void _recv_prepare(nic_3c90x_t *nic)
{
	int oldprod;

	oldprod = rxprod;
	while ((rxprod != rxcons) || !rxpbufs[rxprod])
	{
		int i;
		struct pbuf *p;
		
		if (!rxpbufs[rxprod])
			rxpbufs[rxprod] = p = pbuf_alloc(PBUF_RAW, MAX_RECV_SIZE, PBUF_POOL);
		else {
			outputf("WARNING: 3c90x has pbuf in slot %d", rxprod);
			p = rxpbufs[rxprod];
		}
		
		if (!p)
		{
			outputf("3c90x: out of memory for rx pbuf?");
			break;
		}
		
		rxdescs[rxprod].status = 0;
		rxdescs[rxprod].next = 0;
		for (i = 0; p; p = p->next, i++)
		{
			rxdescs[rxprod].segments[i].addr = v2p(p->payload);
			rxdescs[rxprod].segments[i].len = p->len | (p->next ? 0 : (1 << 31));
		}
		
		/* Hook in the new one after and only after it's been fully set up. */
		rxdescs[(rxprod + RECV_BUFS - 1) % RECV_BUFS].next = v2p(&(rxdescs[rxprod]));
		rxprod = (rxprod + 1) % RECV_BUFS;
	}
	
	if (_inl(nic, regUpListPtr_l) == 0 && rxpbufs[oldprod])	/* Ran out of shit, and got new shit? */
	{
		_outl(nic, regUpListPtr_l, v2p(&rxdescs[oldprod]));
		outputf("3c90x: WARNING: Ran out of rx slots");
	}
	
	_issue_command(nic, cmdStallCtl, 1 /* Unstall upload */);
}

/* _recv polls the ring buffer to see if any packets are available.  If any 
 * are, then eth_recv is called for each available.  _recv returns how many
 * packets it received successfully.  Whether _recv got any packets or not,
 * _recv does not block, and reinitializes the ring buffer with fresh pbufs.
 */
static int _recv(struct nic *_nic)
{
	nic_3c90x_t *nic = (nic_3c90x_t *)_nic;
	int errcode, n = 0;
	struct pbuf *p;
	
	/* Nothing to do? */
	while ((rxdescs[rxcons].status & ((1<<14) | (1<<15))) != 0)
	{
		/** Check for Error (else we have good packet) **/
		if (rxdescs[rxcons].status & (1<<14))
		{
			errcode = rxdescs[rxcons].status;
			if (errcode & (1<<16))
				outputf("3C90X: Rx Overrun (%hX)",errcode>>16);
			else if (errcode & (1<<17))
				outputf("3C90X: Runt Frame (%hX)",errcode>>16);
			else if (errcode & (1<<18))
				outputf("3C90X: Alignment Error (%hX)",errcode>>16);
			else if (errcode & (1<<19))
				outputf("3C90X: CRC Error (%hX)",errcode>>16);
			else if (errcode & (1<<20))
				outputf("3C90X: Oversized Frame (%hX)",errcode>>16);
			else
				outputf("3C90X: Packet error (%hX)",errcode>>16);
		
			p = NULL;	
			pbuf_free(rxpbufs[rxcons]);		/* Bounce the old one before setting it up again. */
		} else {
			p = rxpbufs[rxcons];
			pbuf_realloc(p, rxdescs[rxcons].status & 0x1FFF);	/* Resize the packet to how large it actually is. */
		}
		
		rxpbufs[rxcons] = NULL;
		rxdescs[rxcons].status = 0; 
		rxcons = (rxcons + 1) % RECV_BUFS;
		
		if (p)
		{
			eth_recv(_nic, p);
			n++;
		}
	}

	_recv_prepare(nic);	/* Light the NIC up again. */
	return n;
}

/*** a3c90x_probe: exported routine to probe for the 3c905 card and perform
 *** initialization.  If this routine is called, the pci functions did find the
 *** card.  We just have to init it here.
 ***/
static int _probe(struct pci_dev *pci, void *data)
{
	nic_3c90x_t *nic = &_nic;
	int i, c;
	unsigned short eeprom[0x100];
	unsigned int cfg;
	unsigned int mopt;
	unsigned int mstat;
	unsigned short linktype;
#define	HWADDR_OFFSET	10

	unsigned long ioaddr = 0;
	for (i = 0; i < 6; i++)
		if (pci->bars[i].type == PCI_BAR_IO)
		{
			ioaddr = pci->bars[i].addr;
			break;
		}
		
	if (ioaddr == 0)
	{
		outputf("3c90x: Unable to find I/O address");
		return 0;
	}
    
	/* Power it on */
	pci_write16(pci->bus, pci->dev, pci->fn, 0xE0,
		pci_read16(pci->bus, pci->dev, pci->fn, 0xE0) & ~0x3);
	
	outputf("3c90x: Picked I/O address %04x", ioaddr);
	pci_bother_add(pci);
	nic->nic.ioaddr = ioaddr & ~3;
	nic->nic.irqno = 0;

	nic->ioaddr = ioaddr;
	nic->is3c556 = (pci->did == 0x6055);
	nic->curwnd = 255;
	switch (_read_eeprom(nic, 0x03))
	{
	case 0x9000: /** 10 Base TPO             **/
	case 0x9001: /** 10/100 T4               **/
	case 0x9050: /** 10/100 TPO              **/
	case 0x9051: /** 10 Base Combo           **/
		nic->isBrev = 0;
		break;

	case 0x9004: /** 10 Base TPO             **/
	case 0x9005: /** 10 Base Combo           **/
	case 0x9006: /** 10 Base TPO and Base2   **/
	case 0x900A: /** 10 Base FL              **/
	case 0x9055: /** 10/100 TPO              **/
	case 0x9056: /** 10/100 T4               **/
	case 0x905A: /** 10 Base FX              **/
	default:
		nic->isBrev = 1;
		break;
	}

	/** Load the EEPROM contents **/
	if (nic->isBrev)
		for(i=0;i<=0x20;i++)
			eeprom[i] = _read_eeprom(nic, i);
	else
		for(i=0;i<=0x17;i++)
			eeprom[i] = _read_eeprom(nic, i);

	/** Retrieve the Hardware address and print it on the screen. **/
	nic->nic.hwaddr[0] = eeprom[HWADDR_OFFSET + 0]>>8;
	nic->nic.hwaddr[1] = eeprom[HWADDR_OFFSET + 0]&0xFF;
	nic->nic.hwaddr[2] = eeprom[HWADDR_OFFSET + 1]>>8;
	nic->nic.hwaddr[3] = eeprom[HWADDR_OFFSET + 1]&0xFF;
	nic->nic.hwaddr[4] = eeprom[HWADDR_OFFSET + 2]>>8;
	nic->nic.hwaddr[5] = eeprom[HWADDR_OFFSET + 2]&0xFF;
	outputf("MAC Address = %02x:%02x:%02x:%02x:%02x:%02x",
		nic->nic.hwaddr[0], nic->nic.hwaddr[1],
		nic->nic.hwaddr[2], nic->nic.hwaddr[3],
		nic->nic.hwaddr[4], nic->nic.hwaddr[5]);

	/** 3C556: Invert MII power **/
	if (nic->is3c556) {
		_set_window(nic, winAddressing2);
		_outw(nic, regResetOptions_2_w,
			_inw(nic, regResetOptions_2_w) | 0x4000);
	}

	/* Test if the link is good; if not, bail out */
	_set_window(nic, winDiagnostics4);
	mstat = _inw(nic, regMediaStatus_4_w);
	if((mstat & (1<<11)) == 0) {
		outputf("3c90x: valid link not established");
		return 0;
	}

	/* Program the MAC address into the station address registers */
	_set_window(nic, winAddressing2);
	_outw(nic, regStationAddress_2_3w, htons(eeprom[HWADDR_OFFSET + 0]));
	_outw(nic, regStationAddress_2_3w+2, htons(eeprom[HWADDR_OFFSET + 1]));
	_outw(nic, regStationAddress_2_3w+4, htons(eeprom[HWADDR_OFFSET + 2]));
	_outw(nic, regStationMask_2_3w+0, 0);
	_outw(nic, regStationMask_2_3w+2, 0);
	_outw(nic, regStationMask_2_3w+4, 0);

	/** Read the media options register, print a message and set default
	 ** xcvr.
	 **
	 ** Uses Media Option command on B revision, Reset Option on non-B
	 ** revision cards -- same register address
	 **/
	_set_window(nic, winTxRxOptions3);
	mopt = _inw(nic, regResetMediaOptions_3_w);

	/** mask out VCO bit that is defined as 10baseFL bit on B-rev cards **/
	if (!nic->isBrev)
		mopt &= 0x7F;

	outputf("3c90x: connectors present: ");
	c = 0;
	linktype = 0x0008;
	if (mopt & 0x01)
	{
		outputf("  100Base-T4");
		linktype = 0x0006;
	}
	if (mopt & 0x04)
	{
		outputf("  100Base-FX");
		linktype = 0x0005;
	}
	if (mopt & 0x10)
	{
		outputf("  10Base-2");
		linktype = 0x0003;
	}
	if (mopt & 0x20)
	{
		outputf("  AUI");
		linktype = 0x0001;
	}
	if (mopt & 0x40)
	{
		outputf("  MII");
		linktype = 0x0006;
	}
	if ((mopt & 0xA) == 0xA)
	{
		outputf("  10Base-T / 100Base-TX");
		linktype = 0x0008;
	} else if ((mopt & 0xA) == 0x2) {
		outputf("  100Base-TX");
		linktype = 0x0008;
	} else if ((mopt & 0xA) == 0x8) {
		outputf("  10Base-T");
		linktype = 0x0008;
	}

	/** Determine transceiver type to use, depending on value stored in
	 ** eeprom 0x16
	 **/
	if (nic->isBrev && ((eeprom[0x16] & 0xFF00) == XCVR_MAGIC))
		linktype = eeprom[0x16] & 0x000F;	/* User-defined */
	else if (linktype == 0x0009) {
		if (nic->isBrev)
			outputf("WARNING: MII External MAC Mode only supported on B-revision "
			        "cards!!!!\nFalling Back to MII Mode\n");
		linktype = 0x0006;
	}

	/** enable DC converter for 10-Base-T **/
	if (linktype == 0x0003)
		_issue_command(nic, cmdEnableDcConverter, 0);

	/** Set the link to the type we just determined. **/
	_set_window(nic, winTxRxOptions3);
	cfg = _inl(nic, regInternalConfig_3_l);
	cfg &= ~(0xF<<20);
	cfg |= (linktype<<20);
	_outl(nic, regInternalConfig_3_l, cfg);

	/* Reset and turn on the transmit engine. */
	_issue_command(nic, cmdTxReset, 0);
	if (!nic->isBrev)
		_outb(nic, regTxFreeThresh_b, 0x01);
	_issue_command(nic, cmdTxEnable, 0);

	/* Reset and turn on the receive engine. */
	_issue_command(nic, cmdRxReset, nic->isBrev ? 0x04 : 0x00);
	_issue_command(nic, cmdSetRxFilter, 0x01 + 0x02 + 0x04);	/* Individual, multicast, broadcast */
	_recv_prepare(nic);					/* Set up the ring buffer... */
	_issue_command(nic, cmdRxEnable, 0);	/* ... and light it up. */

	/* Turn on interrupts, and ack any that are hanging out. */
	_issue_command(nic, cmdSetInterruptEnable, 0);
	_issue_command(nic, cmdSetIndicationEnable, 0x0014);
	_issue_command(nic, cmdAcknowledgeInterrupt, 0x661);

	/* Register with lwIP. */
	nic->nic.recv = _recv;
	nic->nic.transmit = _transmit;
	eth_register(&(nic->nic));

	return 1;
}

static struct pci_id _pci_ids[] = {
	/* Original 90x revisions: */
	PCI_ROM(0x10b7, 0x6055, "3c556",	 "3C556"),		/* Huricane */
	PCI_ROM(0x10b7, 0x9000, "3c905-tpo",     "3Com900-TPO"),	/* 10 Base TPO */
	PCI_ROM(0x10b7, 0x9001, "3c905-t4",      "3Com900-Combo"),	/* 10/100 T4 */
	PCI_ROM(0x10b7, 0x9050, "3c905-tpo100",  "3Com905-TX"),		/* 100 Base TX / 10/100 TPO */
	PCI_ROM(0x10b7, 0x9051, "3c905-combo",   "3Com905-T4"),		/* 100 Base T4 / 10 Base Combo */
	/* Newer 90xB revisions: */
	PCI_ROM(0x10b7, 0x9004, "3c905b-tpo",    "3Com900B-TPO"),	/* 10 Base TPO */
	PCI_ROM(0x10b7, 0x9005, "3c905b-combo",  "3Com900B-Combo"),	/* 10 Base Combo */
	PCI_ROM(0x10b7, 0x9006, "3c905b-tpb2",   "3Com900B-2/T"),	/* 10 Base TP and Base2 */
	PCI_ROM(0x10b7, 0x900a, "3c905b-fl",     "3Com900B-FL"),	/* 10 Base FL */
	PCI_ROM(0x10b7, 0x9055, "3c905b-tpo100", "3Com905B-TX"),	/* 10/100 TPO */
	PCI_ROM(0x10b7, 0x9056, "3c905b-t4",     "3Com905B-T4"),	/* 10/100 T4 */
	PCI_ROM(0x10b7, 0x9058, "3c905b-9058",   "3Com905B-9058"),	/* Cyclone 10/100/BNC */
	PCI_ROM(0x10b7, 0x905a, "3c905b-fx",     "3Com905B-FL"),	/* 100 Base FX / 10 Base FX */
	/* Newer 90xC revision: */
	PCI_ROM(0x10b7, 0x9200, "3c905c-tpo",    "3Com905C-TXM"),	/* 10/100 TPO (3C905C-TXM) */
	PCI_ROM(0x10b7, 0x9202, "3c920b-emb-ati", "3c920B-EMB-WNM (ATI Radeon 9100 IGP)"),	/* 3c920B-EMB-WNM (ATI Radeon 9100 IGP) */
	PCI_ROM(0x10b7, 0x9210, "3c920b-emb-wnm","3Com20B-EMB WNM"),
	PCI_ROM(0x10b7, 0x9800, "3c980",         "3Com980-Cyclone"),	/* Cyclone */
	PCI_ROM(0x10b7, 0x9805, "3c9805",        "3Com9805"),		/* Dual Port Server Cyclone */
	PCI_ROM(0x10b7, 0x7646, "3csoho100-tx",  "3CSOHO100-TX"),	/* Hurricane */
	PCI_ROM(0x10b7, 0x4500, "3c450",         "3Com450 HomePNA Tornado"),
	PCI_ROM(0x10b7, 0x1201, "3c982a",        "3Com982A"),
	PCI_ROM(0x10b7, 0x1202, "3c982b",        "3Com982B"),
};

struct pci_driver a3c90x_driver = {
	.name     = "3c90x",
	.probe    = _probe,
	.ids      = _pci_ids,
	.id_count = sizeof(_pci_ids)/sizeof(_pci_ids[0]),
};
