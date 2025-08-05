#ifndef __X86_64_MMU_H__
#define __X86_64_MMU_H__

#include <protocol.h>

void mmu_init(struct memory_map memory_map);

/* bitmap */
void mmu_frame_clear(uintptr_t frame_addr);
void mmu_frame_set(uintptr_t frame_addr);

#endif // !__X86_64_MMU_H__
