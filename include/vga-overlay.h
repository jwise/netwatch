#ifndef VGA_OVERLAY_H
#define VGA_OVERLAY_H

void vga_flush_imm(int enb);

void strblit(char *src, int row, int col);

void dolog(char *s);
void dologf(char *s, ...);
void outlog();

void dump_log(char *buffer);

#endif
