/*
 * 3c90x.c -- This file implements the 3c90x driver for etherboot.  Written
 * by Greg Beeley, Greg.Beeley@LightSys.org.  Modified by Steve Smith,
 * Steve.Smith@Juno.Com. Alignment bug fix Neil Newell (nn@icenoir.net).
 *
 * This program Copyright (C) 1999 LightSys Technology Services, Inc.
 * Portions Copyright (C) 1999 Steve Smith
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
 * --------
 *
 * Program written with the assistance of the 3com documentation for
 * the 3c905B-TX card, as well as with some assistance from the 3c59x
 * driver Donald Becker wrote for the Linux kernel, and with some assistance
 * from the remainder of the Etherboot distribution.
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
 * v2.01    5-26-2003 NN Fixed driver alignment issue which
 *                  caused system lockups if driver structures
 *                  not 8-byte aligned.
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
/** any single transmission fails after 16 collisions or other errors
 ** this is the number of times to retry the transmission -- this should
 ** be plenty
 **/
#define	XMIT_RETRIES	5

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


/*** TX descriptor ***/
typedef struct
    {
    unsigned int	DnNextPtr;
    unsigned int	FrameStartHeader;
    struct {
      unsigned int addr;
      unsigned int len;
    } __attribute ((aligned(8))) segments[64];
    }
    TXD __attribute__ ((aligned(8))); /* 64-bit aligned for bus mastering */

/*** RX descriptor ***/
typedef struct
    {
    unsigned int	UpNextPtr;
    unsigned int	UpPktStatus;
    unsigned int	DataAddr;
    unsigned int	DataLength;
    }
    RXD __attribute__ ((aligned(8))); /* 64-bit aligned for bus mastering */

/*** Global variables ***/
static struct
    {
    unsigned int	is3c556;
    unsigned char	isBrev;
    unsigned char	CurrentWindow;
    unsigned int	IOAddr;
    unsigned char	HWAddr[ETH_ALEN];
    TXD			TransmitDPD;
    RXD			ReceiveUPD;
    }
    INF_3C90X;
static struct nic nic;

#define _outl(v,a) outl((a),(v))
#define _outw(v,a) outw((a),(v))
#define _outb(v,a) outb((a),(v))

static int _issue_command(int ioaddr, int cmd, int param)
{
	outw(ioaddr + regCommandIntStatus_w, (cmd << 11) | param);

	while (inw(ioaddr + regCommandIntStatus_w) & INT_CMDINPROGRESS)
		;

	return 0;
}


/*** a3c90x_internal_SetWindow: selects a register window set.
 ***/
static int _set_window(int ioaddr, int window)
{
	if (INF_3C90X.CurrentWindow == window)
		return 0;

	_issue_command(ioaddr, cmdSelectRegisterWindow, window);
	INF_3C90X.CurrentWindow = window;

	return 0;
}


/*** a3c90x_internal_ReadEeprom - read data from the serial eeprom.
 ***/
static unsigned short
a3c90x_internal_ReadEeprom(int ioaddr, int address)
{
	unsigned short val;

	/** Select correct window **/
        _set_window(INF_3C90X.IOAddr, winEepromBios0);

	/** Make sure the eeprom isn't busy **/
	do
        {
        	int i;
        	for (i = 0; i < 165; i++)
        		inb(0x80);	/* wait 165 usec */
        }
	while(0x8000 & inw(ioaddr + regEepromCommand_0_w));

	/** Read the value. **/
	if (INF_3C90X.is3c556)
		_outw(address + (0x230), ioaddr + regEepromCommand_0_w);
	else
		_outw(address + 0x80, ioaddr + regEepromCommand_0_w);

	do
	{
		int i;
		for (i = 0; i < 165; i++)
			inb(0x80);	/* wait 165 usec */
	}
	while(0x8000 & inw(ioaddr + regEepromCommand_0_w));
	val = inw(ioaddr + regEepromData_0_w);
	
	return val;
}


#ifdef	CFG_3C90X_BOOTROM_FIX
/*** a3c90x_internal_WriteEepromWord - write a physical word of
 *** data to the onboard serial eeprom (not the BIOS prom, but the
 *** nvram in the card that stores, among other things, the MAC
 *** address).
 ***/
static int
a3c90x_internal_WriteEepromWord(int ioaddr, int address, unsigned short value)
    {
	/** Select register window **/
        _set_window(ioaddr, winEepromBios0);

	/** Verify Eeprom not busy **/
	while((1<<15) & inw(ioaddr + regEepromCommand_0_w));

	/** Issue WriteEnable, and wait for completion. **/
	_outw(0x30, ioaddr + regEepromCommand_0_w);
	while((1<<15) & inw(ioaddr + regEepromCommand_0_w));

	/** Issue EraseRegister, and wait for completion. **/
	_outw(address + ((0x03)<<6), ioaddr + regEepromCommand_0_w);
	while((1<<15) & inw(ioaddr + regEepromCommand_0_w));

	/** Send the new data to the eeprom, and wait for completion. **/
	_outw(value, ioaddr + regEepromData_0_w);
	_outw(0x30, ioaddr + regEepromCommand_0_w);
	while((1<<15) & inw(ioaddr + regEepromCommand_0_w));

	/** Burn the new data into the eeprom, and wait for completion. **/
	_outw(address + ((0x01)<<6), ioaddr + regEepromCommand_0_w);
	while((1<<15) & inw(ioaddr + regEepromCommand_0_w));

    return 0;
    }
#endif

#ifdef	CFG_3C90X_BOOTROM_FIX
/*** a3c90x_internal_WriteEeprom - write data to the serial eeprom,
 *** and re-compute the eeprom checksum.
 ***/
static int
a3c90x_internal_WriteEeprom(int ioaddr, int address, unsigned short value)
    {
    int cksum = 0,v;
    int i;
    int maxAddress, cksumAddress;

	if (INF_3C90X.isBrev)
	    {
	    maxAddress=0x1f;
	    cksumAddress=0x20;
	    }
	else
	    {
	    maxAddress=0x16;
	    cksumAddress=0x17;
	    }

	/** Write the value. **/
	if (a3c90x_internal_WriteEepromWord(ioaddr, address, value) == -1)
	    return -1;

	/** Recompute the checksum. **/
	for(i=0;i<=maxAddress;i++)
	    {
	    v = a3c90x_internal_ReadEeprom(ioaddr, i);
	    cksum ^= (v & 0xFF);
	    cksum ^= ((v>>8) & 0xFF);
	    }
	/** Write the checksum to the location in the eeprom **/
	if (a3c90x_internal_WriteEepromWord(ioaddr, cksumAddress, cksum) == -1)
	    return -1;

    return 0;
    }
#endif

/*** a3c90x_reset: exported function that resets the card to its default
 *** state.  This is so the Linux driver can re-set the card up the way
 *** it wants to.  If CFG_3C90X_PRESERVE_XCVR is defined, then the reset will
 *** not alter the selected transceiver that we used to download the boot
 *** image.
 ***/
static void a3c90x_reset(void)
    {
#ifdef	CFG_3C90X_PRESERVE_XCVR
    int cfg;
    /** Read the current InternalConfig value. **/
    _set_window(INF_3C90X.IOAddr, winTxRxOptions3);
    cfg = inl(INF_3C90X.IOAddr + regInternalConfig_3_l);
#endif

    /** Send the reset command to the card **/
    outputf("3c90x: issuing RESET");
    _issue_command(INF_3C90X.IOAddr, cmdGlobalReset, 0);

    /** global reset command resets station mask, non-B revision cards
     ** require explicit reset of values
     **/
    _set_window(INF_3C90X.IOAddr, winAddressing2);
    _outw(0, INF_3C90X.IOAddr + regStationMask_2_3w+0);
    _outw(0, INF_3C90X.IOAddr + regStationMask_2_3w+2);
    _outw(0, INF_3C90X.IOAddr + regStationMask_2_3w+4);

#ifdef	CFG_3C90X_PRESERVE_XCVR
    /** Re-set the original InternalConfig value from before reset **/
    _set_window(INF_3C90X.IOAddr, winTxRxOptions3);
    _outl(cfg, INF_3C90X.IOAddr + regInternalConfig_3_l);

    /** enable DC converter for 10-Base-T **/
    if ((cfg&0x0300) == 0x0300)
	{
	_issue_command(INF_3C90X.IOAddr, cmdEnableDcConverter, 0);
	}
#endif

    /** Issue transmit reset, wait for command completion **/
    _issue_command(INF_3C90X.IOAddr, cmdTxReset, 0);
    if (! INF_3C90X.isBrev)
	_outb(0x01, INF_3C90X.IOAddr + regTxFreeThresh_b);
    _issue_command(INF_3C90X.IOAddr, cmdTxEnable, 0);

    /**
     ** reset of the receiver on B-revision cards re-negotiates the link
     ** takes several seconds (a computer eternity)
     **/
    if (INF_3C90X.isBrev)
	_issue_command(INF_3C90X.IOAddr, cmdRxReset, 0x04);
    else
	_issue_command(INF_3C90X.IOAddr, cmdRxReset, 0x00);
    while (inw(INF_3C90X.IOAddr + regCommandIntStatus_w) & INT_CMDINPROGRESS)
       ;
    _issue_command(INF_3C90X.IOAddr, cmdRxEnable, 0);

    _issue_command(INF_3C90X.IOAddr, cmdSetInterruptEnable, 0);
    /** enable rxComplete and txComplete **/
    _issue_command(INF_3C90X.IOAddr, cmdSetIndicationEnable, 0x0014);
    /** acknowledge any pending status flags **/
    _issue_command(INF_3C90X.IOAddr, cmdAcknowledgeInterrupt, 0x661);

    return;
    }



/*** a3c90x_transmit: exported function that transmits a packet.  Does not
 *** return any particular status.  Parameters are:
 *** dest_addr[6] - destination address, ethernet;
 *** proto - protocol type (ARP, IP, etc);
 *** size - size of the non-header part of the packet that needs transmitted;
 *** pkt - the pointer to the packet data itself.
 ***/
static void
a3c90x_transmit(struct pbuf *p)
{
	unsigned char status;
	static unsigned int stillwaiting = 0;
	unsigned int n, len;

	_issue_command(INF_3C90X.IOAddr, cmdStallCtl, 2 /* Stall download */);

	/** Setup the DPD (download descriptor) **/
	INF_3C90X.TransmitDPD.DnNextPtr = 0;
	len = 0;
	n = 0;
	for (; p; p = p->next)
	{
		INF_3C90X.TransmitDPD.segments[n].addr = (unsigned int)p->payload;
		INF_3C90X.TransmitDPD.segments[n].len = p->len | (p->next ? 0 : (1 << 31));
		len += p->len;
		n++;
	}
	/** set notification for transmission completion (bit 15) **/
	INF_3C90X.TransmitDPD.FrameStartHeader = (len) | 0x8000;
	
	outputf("3c90x: Sending %d byte %d seg packet", len, n);

	/** Send the packet **/
	outl(INF_3C90X.IOAddr + regDnListPtr_l, v2p(&(INF_3C90X.TransmitDPD)));
	_issue_command(INF_3C90X.IOAddr, cmdStallCtl, 3 /* Unstall download */);
		
	oneshot_start_ms(10);
	while((inl(INF_3C90X.IOAddr + regDnListPtr_l) != 0) && oneshot_running())
		;
	if (!oneshot_running())
	{
		outputf("3c90x: Download engine pointer timeout");
		return;
	}

	oneshot_start_ms(10);
	stillwaiting = 1;
	if (stillwaiting)
	{
		while (!(inw(INF_3C90X.IOAddr + regCommandIntStatus_w) & INT_TXCOMPLETE) && oneshot_running())
			;
		if (!(inw(INF_3C90X.IOAddr + regCommandIntStatus_w) & INT_TXCOMPLETE))
		{
			outputf("3c90x: tx timeout? txstat %02x", inb(INF_3C90X.IOAddr + regTxStatus_b));
			outputf("3c90x: Gen sts %04x", inw(INF_3C90X.IOAddr + regCommandIntStatus_w));
		}
		status = inb(INF_3C90X.IOAddr + regTxStatus_b);
		outb(INF_3C90X.IOAddr + regTxStatus_b, 0x00);
		stillwaiting = 0;
	}
		
#if 0		
	/** successful completion (sans "interrupt Requested" bit) **/
	if ((status & 0xbf) == 0x80)
		return;

	outputf("3c90x: Status (%hhX)", status);
	/** check error codes **/
	if (status & 0x02)
	{
		outputf("3c90x: Tx Reclaim Error (%hhX)", status);
		a3c90x_reset();
	} else if (status & 0x04) {
		outputf("3c90x: Tx Status Overflow (%hhX)", status);
		for (i=0; i<32; i++)
			_outb(0x00, INF_3C90X.IOAddr + regTxStatus_b);
		/** must re-enable after max collisions before re-issuing tx **/
		_issue_command(INF_3C90X.IOAddr, cmdTxEnable, 0);
	} else if (status & 0x08) {
		outputf("3c90x: Tx Max Collisions (%hhX)", status);
		/** must re-enable after max collisions before re-issuing tx **/
		_issue_command(INF_3C90X.IOAddr, cmdTxEnable, 0);
	} else if (status & 0x10) {
		outputf("3c90x: Tx Underrun (%hhX)", status);
		a3c90x_reset();
	} else if (status & 0x20) {
		outputf("3c90x: Tx Jabber (%hhX)", status);
		a3c90x_reset();
	} else if ((status & 0x80) != 0x80) {
		outputf("3c90x: Internal Error - Incomplete Transmission (%hhX)", status);
		a3c90x_reset();
	}
#endif
}



/*** a3c90x_poll: exported routine that waits for a certain length of time
 *** for a packet, and if it sees none, returns 0.  This routine should
 *** copy the packet to nic->packet if it gets a packet and set the size
 *** in nic->packetlen.  Return 1 if a packet was found.
 ***/
static int
a3c90x_poll(struct nic *nic, int retrieve)
    {
    int i, errcode;

    if (!(inw(INF_3C90X.IOAddr + regCommandIntStatus_w)&0x0010))
	{
	return 0;
	}

    if ( ! retrieve ) return 1;

    /** we don't need to acknowledge rxComplete -- the upload engine
     ** does it for us.
     **/

    /** Build the up-load descriptor **/
    INF_3C90X.ReceiveUPD.UpNextPtr = 0;
    INF_3C90X.ReceiveUPD.UpPktStatus = 0;
    INF_3C90X.ReceiveUPD.DataAddr = v2p(nic->packet);
    INF_3C90X.ReceiveUPD.DataLength = 1536 + (1<<31);

    /** Submit the upload descriptor to the NIC **/
    _outl(v2p(&(INF_3C90X.ReceiveUPD)),
         INF_3C90X.IOAddr + regUpListPtr_l);

    /** Wait for upload completion (upComplete(15) or upError (14)) **/
    for(i=0;i<40000;i++);
    while((INF_3C90X.ReceiveUPD.UpPktStatus & ((1<<14) | (1<<15))) == 0)
	for(i=0;i<40000;i++);

    /** Check for Error (else we have good packet) **/
    if (INF_3C90X.ReceiveUPD.UpPktStatus & (1<<14))
	{
	errcode = INF_3C90X.ReceiveUPD.UpPktStatus;
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
	return 0;
	}

    /** Ok, got packet.  Set length in nic->packetlen. **/
    nic->packetlen = (INF_3C90X.ReceiveUPD.UpPktStatus & 0x1FFF);

    return 1;
    }



/*** a3c90x_disable: exported routine to disable the card.  What's this for?
 *** the eepro100.c driver didn't have one, so I just left this one empty too.
 *** Ideas anyone?
 *** Must turn off receiver at least so stray packets will not corrupt memory
 *** [Ken]
 ***/
void a3c90x_disable(struct dev *dev)
{
	/* reset and disable merge */
	a3c90x_reset();
	/* Disable the receiver and transmitter. */
	_outw(cmdRxDisable, INF_3C90X.IOAddr + regCommandIntStatus_w);
	_outw(cmdTxDisable, INF_3C90X.IOAddr + regCommandIntStatus_w);
}


/*** a3c90x_probe: exported routine to probe for the 3c905 card and perform
 *** initialization.  If this routine is called, the pci functions did find the
 *** card.  We just have to init it here.
 ***/
static int a3c90x_probe(struct pci_dev * pci, void * data)
{
    INF_3C90X.is3c556 = (pci->did == 0x6055);
 
    int i, c;
    unsigned short eeprom[0x100];
    unsigned int cfg;
    unsigned int mopt;
    unsigned int mstat;
    unsigned short linktype;
#define	HWADDR_OFFSET	10

    unsigned long ioaddr = 0;
    for (i = 0; i < 6; i++) {
        if (pci->bars[i].type == PCI_BAR_IO) {
            ioaddr = pci->bars[i].addr;
            break;
        }
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
    nic.ioaddr = ioaddr & ~3;
    nic.irqno = 0;

    INF_3C90X.IOAddr = ioaddr;
    INF_3C90X.CurrentWindow = 255;
    switch (a3c90x_internal_ReadEeprom(INF_3C90X.IOAddr, 0x03))
	{
	case 0x9000: /** 10 Base TPO             **/
	case 0x9001: /** 10/100 T4               **/
	case 0x9050: /** 10/100 TPO              **/
	case 0x9051: /** 10 Base Combo           **/
		INF_3C90X.isBrev = 0;
		break;

	case 0x9004: /** 10 Base TPO             **/
	case 0x9005: /** 10 Base Combo           **/
	case 0x9006: /** 10 Base TPO and Base2   **/
	case 0x900A: /** 10 Base FL              **/
	case 0x9055: /** 10/100 TPO              **/
	case 0x9056: /** 10/100 T4               **/
	case 0x905A: /** 10 Base FX              **/
	default:
		INF_3C90X.isBrev = 1;
		break;
	}

    /** Load the EEPROM contents **/
    if (INF_3C90X.isBrev)
	{
	for(i=0;i<=/*0x20*/0x7F;i++)
	    {
	    eeprom[i] = a3c90x_internal_ReadEeprom(INF_3C90X.IOAddr, i);
	    }

#ifdef	CFG_3C90X_BOOTROM_FIX
	/** Set xcvrSelect in InternalConfig in eeprom. **/
	/* only necessary for 3c905b revision cards with boot PROM bug!!! */
	a3c90x_internal_WriteEeprom(INF_3C90X.IOAddr, 0x13, 0x0160);
#endif

#ifdef	CFG_3C90X_XCVR
	if (CFG_3C90X_XCVR == 255)
	    {
	    /** Clear the LanWorks register **/
	    a3c90x_internal_WriteEeprom(INF_3C90X.IOAddr, 0x16, 0);
	    }
	else
	    {
	    /** Set the selected permanent-xcvrSelect in the
	     ** LanWorks register
	     **/
	    a3c90x_internal_WriteEeprom(INF_3C90X.IOAddr, 0x16,
	                    XCVR_MAGIC + ((CFG_3C90X_XCVR) & 0x000F));
	    }
#endif
	}
    else
	{
	for(i=0;i<=/*0x17*/0x7F;i++)
	    {
	    eeprom[i] = a3c90x_internal_ReadEeprom(INF_3C90X.IOAddr, i);
	    }
	}

    /** Print identification message **/
#ifdef	CFG_3C90X_BOOTROM_FIX
    if (INF_3C90X.isBrev)
        {
        outputf("NOTE: 3c905b bootrom fix enabled; has side "
	   "effects.  See 3c90x.txt for info.");
	}
#endif

    /** Retrieve the Hardware address and print it on the screen. **/
    INF_3C90X.HWAddr[0] = eeprom[HWADDR_OFFSET + 0]>>8;
    INF_3C90X.HWAddr[1] = eeprom[HWADDR_OFFSET + 0]&0xFF;
    INF_3C90X.HWAddr[2] = eeprom[HWADDR_OFFSET + 1]>>8;
    INF_3C90X.HWAddr[3] = eeprom[HWADDR_OFFSET + 1]&0xFF;
    INF_3C90X.HWAddr[4] = eeprom[HWADDR_OFFSET + 2]>>8;
    INF_3C90X.HWAddr[5] = eeprom[HWADDR_OFFSET + 2]&0xFF;
    outputf("MAC Address = %02x:%02x:%02x:%02x:%02x:%02x",
    	INF_3C90X.HWAddr[0],
    	INF_3C90X.HWAddr[1],
    	INF_3C90X.HWAddr[2],
    	INF_3C90X.HWAddr[3],
    	INF_3C90X.HWAddr[4],
    	INF_3C90X.HWAddr[5]);

    /** 3C556: Invert MII power **/
    if (INF_3C90X.is3c556) {
	unsigned int tmp;
	_set_window(INF_3C90X.IOAddr, winAddressing2);
	tmp = inw(INF_3C90X.IOAddr + regResetOptions_2_w);
	tmp |= 0x4000;
	_outw(tmp, INF_3C90X.IOAddr + regResetOptions_2_w);
    }

    /* Test if the link is good, if not continue */
    _set_window(INF_3C90X.IOAddr, winDiagnostics4);
    mstat = inw(INF_3C90X.IOAddr + regMediaStatus_4_w);
    if((mstat & (1<<11)) == 0) {
	outputf("Valid link not established");
	return 0;
    }

    /** Program the MAC address into the station address registers **/
    _set_window(INF_3C90X.IOAddr, winAddressing2);
    _outw(htons(eeprom[HWADDR_OFFSET + 0]), INF_3C90X.IOAddr + regStationAddress_2_3w);
    _outw(htons(eeprom[HWADDR_OFFSET + 1]), INF_3C90X.IOAddr + regStationAddress_2_3w+2);
    _outw(htons(eeprom[HWADDR_OFFSET + 2]), INF_3C90X.IOAddr + regStationAddress_2_3w+4);
    _outw(0, INF_3C90X.IOAddr + regStationMask_2_3w+0);
    _outw(0, INF_3C90X.IOAddr + regStationMask_2_3w+2);
    _outw(0, INF_3C90X.IOAddr + regStationMask_2_3w+4);

    /** Fill in our entry in the etherboot arp table **/
/* XXX ? for lwip? 
    for(i=0;i<ETH_ALEN;i++)
	nic.node_addr[i] = (eeprom[HWADDR_OFFSET + i/2] >> (8*((i&1)^1))) & 0xff;
*/

    /** Read the media options register, print a message and set default
     ** xcvr.
     **
     ** Uses Media Option command on B revision, Reset Option on non-B
     ** revision cards -- same register address
     **/
    _set_window(INF_3C90X.IOAddr, winTxRxOptions3);
    mopt = inw(INF_3C90X.IOAddr + regResetMediaOptions_3_w);

    /** mask out VCO bit that is defined as 10baseFL bit on B-rev cards **/
    if (! INF_3C90X.isBrev)
	{
	mopt &= 0x7F;
	}

    outputf("Connectors present: ");
    c = 0;
    linktype = 0x0008;
    if (mopt & 0x01)
	{
	outputf("%s100Base-T4",(c++)?", ":"");
	linktype = 0x0006;
	}
    if (mopt & 0x04)
	{
	outputf("%s100Base-FX",(c++)?", ":"");
	linktype = 0x0005;
	}
    if (mopt & 0x10)
	{
	outputf("%s10Base-2",(c++)?", ":"");
	linktype = 0x0003;
	}
    if (mopt & 0x20)
	{
	outputf("%sAUI",(c++)?", ":"");
	linktype = 0x0001;
	}
    if (mopt & 0x40)
	{
	outputf("%sMII",(c++)?", ":"");
	linktype = 0x0006;
	}
    if ((mopt & 0xA) == 0xA)
	{
	outputf("%s10Base-T / 100Base-TX",(c++)?", ":"");
	linktype = 0x0008;
	}
    else if ((mopt & 0xA) == 0x2)
	{
	outputf("%s100Base-TX",(c++)?", ":"");
	linktype = 0x0008;
	}
    else if ((mopt & 0xA) == 0x8)
	{
	outputf("%s10Base-T",(c++)?", ":"");
	linktype = 0x0008;
	}
    outputf(".");

    /** Determine transceiver type to use, depending on value stored in
     ** eeprom 0x16
     **/
    if (INF_3C90X.isBrev)
	{
	if ((eeprom[0x16] & 0xFF00) == XCVR_MAGIC)
	    {
	    /** User-defined **/
	    linktype = eeprom[0x16] & 0x000F;
	    }
	}
    else
	{
#ifdef	CFG_3C90X_XCVR
	    if (CFG_3C90X_XCVR != 255)
		linktype = CFG_3C90X_XCVR;
#endif	/* CFG_3C90X_XCVR */

	    /** I don't know what MII MAC only mode is!!! **/
	    if (linktype == 0x0009)
		{
		if (INF_3C90X.isBrev)
			outputf("WARNING: MII External MAC Mode only supported on B-revision "
			       "cards!!!!\nFalling Back to MII Mode\n");
		linktype = 0x0006;
		}
	}

    /** enable DC converter for 10-Base-T **/
    if (linktype == 0x0003)
	{
	_issue_command(INF_3C90X.IOAddr, cmdEnableDcConverter, 0);
	}

    /** Set the link to the type we just determined. **/
    _set_window(INF_3C90X.IOAddr, winTxRxOptions3);
    cfg = inl(INF_3C90X.IOAddr + regInternalConfig_3_l);
    cfg &= ~(0xF<<20);
    cfg |= (linktype<<20);
    _outl(cfg, INF_3C90X.IOAddr + regInternalConfig_3_l);

    /** Now that we set the xcvr type, reset the Tx and Rx, re-enable. **/
    _issue_command(INF_3C90X.IOAddr, cmdTxReset, 0);
    if (!INF_3C90X.isBrev)
	_outb(0x01, INF_3C90X.IOAddr + regTxFreeThresh_b);

    _issue_command(INF_3C90X.IOAddr, cmdTxEnable, 0);

    /**
     ** reset of the receiver on B-revision cards re-negotiates the link
     ** takes several seconds (a computer eternity)
     **/
    if (INF_3C90X.isBrev)
	_issue_command(INF_3C90X.IOAddr, cmdRxReset, 0x04);
    else
	_issue_command(INF_3C90X.IOAddr, cmdRxReset, 0x00);

    /** Set the RX filter = receive only individual pkts & multicast & bcast. **/
    _issue_command(INF_3C90X.IOAddr, cmdSetRxFilter, 0x01 + 0x02 + 0x04);
    _issue_command(INF_3C90X.IOAddr, cmdRxEnable, 0);


    /**
     ** set Indication and Interrupt flags , acknowledge any IRQ's
     **/
    _issue_command(INF_3C90X.IOAddr, cmdSetInterruptEnable, 0);
    _issue_command(INF_3C90X.IOAddr, cmdSetIndicationEnable, 0x0014);
    _issue_command(INF_3C90X.IOAddr, cmdAcknowledgeInterrupt, 0x661);

    /* * Set our exported functions **/
    nic.poll     = a3c90x_poll;
    nic.transmit = a3c90x_transmit;
    memcpy(nic.hwaddr, INF_3C90X.HWAddr, 6);
    eth_register(&nic);

    return 1;
}

static struct pci_id a3c90x_nics[] = {
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
	.name     = "3C90X",
	.probe    = a3c90x_probe,
	.ids      = a3c90x_nics,
	.id_count = sizeof(a3c90x_nics)/sizeof(a3c90x_nics[0]),
};
