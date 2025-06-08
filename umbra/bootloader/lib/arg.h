#ifndef __ARG_H__
#define __ARG_H__

/* what a cheater */
typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, type) __builtin_va_arg(v, type)

#endif // !__ARG_H__
