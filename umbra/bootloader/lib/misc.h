#ifndef __MISC_H__
#define __MISC_H__

#include <types.h>

#define DIV_ROUNDUP(a, b) ({ \
    __auto_type DIV_ROUNDUP_a = (a); \
    __auto_type DIV_ROUNDUP_b = (b); \
    (DIV_ROUNDUP_a + (DIV_ROUNDUP_b - 1)) / DIV_ROUNDUP_b; \
})

#define SIZEOF_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);
char *itoa(uint32_t value, char* result, uint8_t base);
int strncmp(const char *a, const char *b, size_t n);
int strcmp(const char *a, const char *b);
int toupper(int c);
#endif // !__MISC_H__
