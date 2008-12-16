/* keyboard.h
 * Keyboard injection definitions
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void kbd_inject_keysym(uint32_t k, int downflag);

unsigned char kbd_get_injected_scancode();
int kbd_has_injected_scancode();

extern int kbd_mode;

#endif
