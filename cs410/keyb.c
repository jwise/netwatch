#include <io.h>
#include "keyhelp.h"
#include "../net/net.h"
#include <output.h>

/* Takes over the i8042... oh well.  If we ever fully emulate the i8042,
 * then we'll patch into that.
 */

#define KEYBOARD_PORT 0x60
#define KEYB_STATUS_PORT 0x64
#define KEYB_OBF 0x1

int getchar()
{
	kh_type key;
	unsigned char scancode;
	
	/* Ignore accesses -- XXX hack */
	outl(0x840, 0x0);
	outl(0x848, 0x0);
	
	do
	{
		while (!(inb(KEYB_STATUS_PORT) & KEYB_OBF))
			eth_poll();
		
		scancode = inb(KEYBOARD_PORT);
		outputf("scancode: %02x", scancode);
		key = process_scancode(scancode);
	} while (!KH_HASDATA(key) || !KH_ISMAKE(key));
	
	/* XXX */
	outl(0x840, 0x0);
	outl(0x844, 0x1000);
	outl(0x848, 0x1000);
	
	return KH_GETCHAR(key);
}
