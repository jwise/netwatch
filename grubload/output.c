#include <stdarg.h>
#include <minilib.h>
#include <console.h>
#include <output.h>
#include <smram.h>

#define OUTBLEN 160

static void safeputs(const char *s)
{
	unsigned long old = smram_save_state();
	smram_aseg_set_state(SMRAM_ASEG_SMMONLY);
	puts(s);
	smram_restore_state(old);
}
void (*output)(const char *s) = safeputs;

static void miniprintf(const char *fmt, ...)
{
	va_list va;
	char b[OUTBLEN+1];
	
	va_start(va, fmt);
	vsnprintf(b, OUTBLEN, fmt, va);
	va_end(va);
	
	output(b);
	putbyte('\n');
}

void (*outputf)(const char *s, ...) = miniprintf;
