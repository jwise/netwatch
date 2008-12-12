#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void kbd_inject_keysym(uint32_t k, int downflag);

unsigned char kbd_get_injected_scancode();
int kbd_has_injected_scancode();

extern int kbd_mode;

#endif
