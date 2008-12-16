/* output.h
 * Prototypes for timer handling routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef TIMER_H
#define TIMER_H

void oneshot_start_ms(unsigned long milliseconds);
int oneshot_running(void);

#endif /* TIMER_H */
