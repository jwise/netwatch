/** @file console.c 
 *  @brief A console driver.
 *
 *  @author Joshua Wise (jwise) <joshua@joshuawise.com>
 *  @bug None known
 */

#include <console.h>
#include "console-ext.h"
#include <io.h>

#define POS_IS_VALID(row, col) ((row) >= 0 && (row) < CONSOLE_HEIGHT && (col) >= 0 && (col) < CONSOLE_WIDTH)

/** @brief A structure describing a console that is backed by memory.
 * 
 *  In the mode of designing for extensibility, all of the console driver's
 *  state is encapsulated in a 'struct console', which would ostensibly make
 *  it easier to create virtual consoles later.  All of the console driver's
 *  workings touch this struct; ideally, there are no references to
 *  CONSOLE_MEM_BASE or other hardware outside of this.  In practice, this
 *  isn't quite the case (see update_cursor()).... but we're close.
 */
struct console {
	int row;		/**< The current row. */
	int col;		/**< The current column. */
	unsigned char attr;	/**< The current color/attribute. */
	unsigned char *base;	/**< The current base to write data to.
				 *   Might not be equal to physbase if we're
				 *   in a different virtual console right
				 *   now or we have requested a backbuffer!
				 */
	unsigned char *physbase;	/**< The physical memory base for
					 *   this console.
					 */
	int showcursor;		/**< Whether the cursor should be shown
				 *   by update_cursor().
				 */
	int initialized;	/**< Whether the console has been
				 *   initialized. Functions should check
				 *   this and call clear_console() before
				 *   running if this is zero.
				 */
	int backbuffer;		/**< Whether we're currently writing to a
				 *   backbuffer instead of the physical
				 *   video memory.
				 */
};

/** @brief The structure describing the one console on the system. Should
 *         not be touched by anyone but the initializer for curcons!
 */
static struct console cons = {
	.row = 0,
	.col = 0,
	.attr = FGND_LGRAY,
	.base = (unsigned char *)CONSOLE_MEM_BASE,
	.physbase = (unsigned char *)CONSOLE_MEM_BASE,
	.showcursor = 1,
	.initialized = 0,
	.backbuffer = 0,
};

/** @brief The current console that all console.c operations work on. */
static struct console *curcons = &cons;

/** @brief Makes sure that the VGA cursor matches with the console.c's idea
 *         of where the cursor should be.
 *
 *  update_cursor would be trivial, but for a few important checks.  In
 *  particular, it won't touch the cursor at all if we're backbuffered
 *  (since that would make the cursor fly around a screen that isn't
 *  actually being updated), and if the cursor is hidden, it sets it to an
 *  out-of-bounds segment to make sure that it's actually hidden.
 */
static void update_cursor()
{
	if (curcons->backbuffer)
		return;
	if (curcons->showcursor)
	{
		unsigned short addr = (curcons->row * CONSOLE_WIDTH + curcons->col);
		outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
		outb(CRTC_DATA_REG, (addr >> 8));
		outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
		outb(CRTC_DATA_REG, addr & 0xFF);
	} else {
		outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
		outb(CRTC_DATA_REG, 255	/* invalid */);
		outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
		outb(CRTC_DATA_REG, 255	/* invalid */);
	}
}

/** @brief Redirects console writes to a backbuffer.
 *
 *  Verifies that the console is not already backbuffered.  If it's not, it
 *  allocates a backbuffer, copies the current console into the backbuffer,
 *  and sets the backbuffered flag.
 *
 *  This isn't just theoretical, by the way.  The game screen's timer causes
 *  it to repaint every frame (for lack of a better way to do it), which is
 *  fine in qemu (which is fast), but causes severe flicker in simics.  This
 *  backbuffering logic seems to have alleviated the flicker.
 *
 *  @see cons_debackbuffer
 */
/*void cons_backbuffer()
{
	if (!curcons->initialized)
		clear_console();
	if (curcons->backbuffer)
		return;
	curcons->base = malloc(CONSOLE_WIDTH * CONSOLE_HEIGHT * 2);
	memcpy(curcons->base, curcons->physbase, CONSOLE_WIDTH * CONSOLE_HEIGHT * 2);
	curcons->backbuffer = 1;
}*/

/** @brief Turns off the backbuffer.
 *
 *  Verifies that we are currently backbuffered.  If so, copies the
 *  backbuffer into video memory, frees the backbuffer, sets the pointer
 *  back to video memory, clears the backbuffered flag, and updates the
 *  hardware cursor.
 *
 *  @see cons_backbuffer
 */
/*void cons_debackbuffer()
{
	if (!curcons->initialized)
		clear_console();
	if (!curcons->backbuffer)
		return;
	memcpy(curcons->physbase, curcons->base, CONSOLE_WIDTH * CONSOLE_HEIGHT * 2);
	free(curcons->base);
	curcons->base = curcons->physbase;
	curcons->backbuffer = 0;
	update_cursor();
}*/

int putbyte(char ch)
{
	if (!curcons->initialized)
		clear_console();
	
	/* Make sure to handle special cases nicely.*/
	switch(ch)
	{
	case '\n':
		curcons->row++;
		if (curcons->row >= CONSOLE_HEIGHT)	/* Moving off the end? Scroll. */
		{
			int c;
			memmove(curcons->base, curcons->base + 2*CONSOLE_WIDTH, 2*CONSOLE_WIDTH*(CONSOLE_HEIGHT-1));
			curcons->row--;
			for (c=0; c<CONSOLE_WIDTH; c++)	/* Clear the newly blank bottom line. */
			{
				curcons->base[(curcons->row * CONSOLE_WIDTH + c) * 2] = ' ';
				curcons->base[(curcons->row * CONSOLE_WIDTH + c) * 2 + 1] = curcons->attr;
			}
		}
		// fall through
	case '\r':
		curcons->col = 0;
		update_cursor();
		break;
	case '\b':
		if (curcons->col)
		{
			curcons->col--;
			curcons->base[(curcons->row*CONSOLE_WIDTH + curcons->col) * 2] = ' ';
		}
		update_cursor();
		break;
	default:
		curcons->base[(curcons->row*CONSOLE_WIDTH + curcons->col) * 2] = ch;
		curcons->base[(curcons->row*CONSOLE_WIDTH + curcons->col) * 2 + 1] = curcons->attr;
		curcons->col++;
		if (curcons->col >= CONSOLE_WIDTH)
			putbyte('\n');
		update_cursor();
	}
	return ch;
}

void putbytes(const char *s, int len)
{
	if (!curcons->initialized)
		clear_console();
	
	while (len--)
		putbyte(*(s++));
}

int set_term_color(int color)
{
	if (!curcons->initialized)
		clear_console();
	
	curcons->attr = (unsigned char)color;
	return 0;
}

void get_term_color(int *color)
{
	if (!curcons->initialized)
		clear_console();
	
	*color = (int)curcons->attr;
}

int set_cursor(int row, int col)
{
	if (!curcons->initialized)
		clear_console();
	if (!POS_IS_VALID(row, col))
		return -1;
	curcons->row = row;
	curcons->col = col;
	update_cursor();
	return 0;
}

void get_cursor(int *row, int *col)
{
	if (!curcons->initialized)
		clear_console();
	*row = curcons->row;
	*col = curcons->col;
}

void hide_cursor()
{
	if (!curcons->initialized)
		clear_console();
	curcons->showcursor = 0;
	update_cursor();
}

void show_cursor()
{
	if (!curcons->initialized)
		clear_console();
	curcons->showcursor = 1;
	update_cursor();
}

void clear_console()
{
	int i;
	curcons->initialized = 1;
	curcons->row = 0;
	curcons->col = 0;
	for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
		curcons->base[i*2] = ' ';
		curcons->base[i*2+1] = FGND_LGRAY;
	}
	update_cursor();
}

void draw_char(int row, int col, int ch, int color)
{
	if (!POS_IS_VALID(row, col))
		return;
	curcons->base[2 * (CONSOLE_WIDTH * row + col)] = (unsigned char)ch;
	curcons->base[2 * (CONSOLE_WIDTH * row + col)+1] = (unsigned char)color;
}

char get_char(int row, int col)
{
	if (!POS_IS_VALID(row, col))
		return 0;
	return curcons->base[2 * (CONSOLE_WIDTH * row + col)];
}
