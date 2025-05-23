#ifndef __REAL_H__
#define __REAL_H__

#include <types.h>

#define rm_desegment(seg, off) (((uint32_t)(seg) << 4) + (uint32_t)(off))

/* The scratch buffer used in real mode code.  */
#define SCRATCH_ADDR 0x68000
#define SCRATCH_SEG (SCRATCH_ADDR >> 4)
#define SCRATCH_SIZE 0x9000

struct int_regs {
  uint32_t eax;
  uint16_t es;
  uint16_t ds;
  uint16_t flags;
  uint16_t dummy;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edi;
  uint32_t esi;
  uint32_t edx;
};

struct rm_regs {
    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;
    uint32_t eflags;
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
} __attribute__((packed));

void rm_int(uint8_t intnb, struct int_regs *regs)
	__attribute__((regparm(3)));

#endif // !__REAL_H__
