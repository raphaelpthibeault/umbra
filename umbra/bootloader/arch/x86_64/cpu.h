#ifndef __CPU_H__
#define __CPU_H__

#include <types.h>

/* port outb */
static inline void 
outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

/* port inb */
static inline uint8_t 
inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( 
			"inb %w1, %b0"
			: "=a"(ret)
			: "Nd"(port)
			: "memory"
		);
    return ret;
}

/* memory inw */
static inline uint16_t
mminw(uintptr_t addr) 
{
	uint16_t ret;
	__asm__ volatile (
		"movw (%1), %0"
		: "=r"(ret)
		: "r"(addr)
		: "memory"
	);
	return ret;
}

/* memory inb */
static inline uint8_t
mminb(uintptr_t addr) 
{
	uint8_t ret;
	__asm__ volatile (
		"movb (%1), %0"
		: "=r"(ret)
		: "r"(addr)
		: "memory"
	);
	return ret;
}

#endif // !__CPU_H__
