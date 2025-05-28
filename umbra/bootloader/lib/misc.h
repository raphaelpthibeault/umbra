#ifndef __MISC_H__
#define __MISC_H__

#include <types.h>

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
char *itoa(uint32_t value, char* result, uint8_t base);

#endif // !__MISC_H__
