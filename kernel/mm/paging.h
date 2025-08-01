#ifndef __MM_PAGER_H__
#define __MM_PAGER_H__

#include <types.h>
#include <mm/memory.h>

/* to provide arch-specific implementations */
#if defined (__x86_64__) || defined (__i386__)

#include <arch/x86_64/paging.h>

#else
#error Unsupported architecture
#endif

/* COMMON */

#endif // !__MM_PAGER_H__
