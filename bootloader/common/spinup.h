#ifndef __COMMON_SPINUP_H__
#define __COMMON_SPINUP_H__

#include <types.h>
#include <lib/arg.h>

#if defined (__x86_64__) || defined (__i386__)
noreturn void spinup(void *fnptr, int args, ...);
#else
#error Unsupported architecture
#endif


#endif // !__COMMON_SPINUP_H__
