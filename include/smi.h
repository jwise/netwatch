/* smi.h
 * Prototypes for generic SMI handling routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef SMI_H
#define SMI_H

extern void smi_disable();	/* akin to cli / sti */
extern void smi_enable();

extern void smi_poll();
extern unsigned long smi_status();	/* Architecturally defined; for debugging only. */

typedef enum {
	SMI_EVENT_FAST_TIMER = 0,
	SMI_EVENT_DEVTRAP_KBC,
	SMI_EVENT_GBL_RLS,
	SMI_EVENT_MAX
} smi_event_t;


typedef void (*smi_handler_t)(smi_event_t);

#define SMI_HANDLER_NONE	((smi_handler_t)0)
#define SMI_HANDLER_IGNORE	((smi_handler_t)-1)

extern int smi_register_handler(smi_event_t ev, smi_handler_t hnd);
extern int smi_enable_event(smi_event_t ev);
extern int smi_disable_event(smi_event_t ev);

#endif
