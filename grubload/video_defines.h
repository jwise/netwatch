/*
 * Name: video_defines.h
 * Date: January 15th 2003
 * Author: Steve Muckle <smuckle@andrew.cmu.edu>
 *
 * Description:
 * 
 * Important values for console output.
 */

#ifndef _VIDEO_DEFINES_H
#define _VIDEO_DEFINES_H

#define CONSOLE_MEM_BASE 0xB8000
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

/* Bits 3:0 are the foreground color, bits 6:4 are the
   background color, and bit 7 specifies blink (annoying). */
#define FGND_BLACK 0x0
#define FGND_BLUE  0x1
#define FGND_GREEN 0x2
#define FGND_CYAN  0x3
#define FGND_RED   0x4
#define FGND_MAG   0x5
#define FGND_BRWN  0x6
#define FGND_LGRAY 0x7 /* Light gray. */
#define FGND_DGRAY 0x8 /* Dark gray. */
#define FGND_BBLUE 0x9 /* Bright blue. */
#define FGND_BGRN  0xA /* Bright green. */
#define FGND_BCYAN 0xB /* Bright cyan. */
#define FGND_PINK  0xC
#define FGND_BMAG  0xD /* Bright magenta. */
#define FGND_YLLW  0xE
#define FGND_WHITE 0xF

#define BGND_BLACK 0x00
#define BGND_BLUE  0x10
#define BGND_GREEN 0x20
#define BGND_CYAN  0x30
#define BGND_RED   0x40
#define BGND_MAG   0x50
#define BGND_BRWN  0x60
#define BGND_LGRAY 0x70 /* Light gray. */

#define BLINK      0x80 /* Annoying. */

/* --- CRTC Register Manipulation --- */
#define CRTC_IDX_REG 0x3d4
#define CRTC_DATA_REG 0x3d5
#define CRTC_CURSOR_LSB_IDX 15
#define CRTC_CURSOR_MSB_IDX 14

#endif
