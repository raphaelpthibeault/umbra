#ifndef __MEMORY_REFERENCES_H__
#define __MEMORY_REFERENCES_H__

#define KERNEL_VMA 0xffffffff80000000

#ifdef __ASSEMBLER__
#define V2P(addr) ((addr) - KERNEL_VMA)
#define P2V(addr) ((addr) + KERNEL_VMA)
#else
#include <types.h>
#define V2P(addr) ((uintptr_t)(addr) & ~KERNEL_VMA)
#define P2V(addr) ((uintptr_t(addr) | KERNEL_VMA)
#endif


#endif // !__MEMORY_REFERENCES_H__
