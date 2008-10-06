#ifndef KEYBOARD_H
#define KEYBOARD_H

void kbd_inject_key(unsigned char k);

unsigned char kbd_get_injected_scancode();

extern int kbd_mode;

#endif
