#ifndef SMI_H
#define SMI_H

extern void smi_disable();	/* akin to cli / sti */
extern void smi_enable();

extern void smi_poll();
extern unsigned long smi_status();	/* Architecturally defined; for debugging only. */

#endif
