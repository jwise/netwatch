#ifndef _STDARG_H_
#define _STDARG_H_

/* This is awful, but really these are compiler intrinsics, so we use the
 * GNU compiler intrinsics.
 */

#ifdef __GNUC__
typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)
#else
#error "Don't know how to use varargs not on GNUC, sorry."
#endif

#endif
