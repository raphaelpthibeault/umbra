#ifndef __MEMORY_REFERENCES_H__
#define __MEMORY_REFERENCES_H__

#define KERNEL_VMA 0xffffffff80000000
#define USER_VMA 0xffff800000000000
#define IDT_LENGTH 0x1000
#define GDT_LENGTH 0x1000

#define KERNEL_CODE 0x20
#define KERNEL_STACK 0x28
#define KERNEL_DATA 0x30
#define USER_DATA 0x3b
#define USER_STACK 0x43
#define USER_CODE 0x4b

#ifdef __ASSEMBLER__
#define V2P(addr) ((addr) - KERNEL_VMA)
#define P2V(addr) ((addr) + KERNEL_VMA)
#else
#include <types.h>
#define V2P(addr) ((uintptr_t)(addr) - KERNEL_VMA)
#define P2V(addr) ((uintptr_t)(addr) | KERNEL_VMA)
#endif


#endif // !__MEMORY_REFERENCES_H__
