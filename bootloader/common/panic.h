#ifndef __COMMON_PANIC_H__
#define __COMMON_PANIC_H__

#include <types.h>
#include <lib/arg.h>

noreturn void panic(const char *fmt, ...);

#endif // !__COMMON_PANIC_H__
