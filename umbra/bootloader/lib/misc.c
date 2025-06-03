#include <lib/misc.h>

void * 
memset(void *dest, int c, size_t n) 
{
	uint8_t *pdest = dest;	
	
	for (size_t i = 0; i < n; ++i) {
		pdest[i] = (uint8_t)c;
	}

	return dest;
}

void *
memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *pdest = (uint8_t *)dest;
	const uint8_t *psrc = (const uint8_t *)src;

	for (size_t i = 0; i < n; ++i) {
		pdest[i] = psrc[i];
	}

	return dest;
}

int 
memcmp(const void *a, const void *b, size_t n)
{
	uint8_t *pa, *pb;
	pa = (uint8_t *)a;
	pb = (uint8_t *)b;

	// obviously unsafe but checking if pa or pb isn't really this function's responsibility
	for (size_t i = 0; i < n; ++i) {
		if (pa[i] != pb[i]) {
			return pa < pb ? -1 : 1;
		}
	}

	return 0;
}

char* 
itoa(uint32_t value, char* result, uint8_t base) 
{
	// check that the base is valid
	if (base < 2 || base > 36) { 
		*result = '\0'; return result; 
	}

	char* ptr = result, *ptr1 = result, tmp_char;
	uint64_t tmp_value;


	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	/*
	// apply negative sign
	if (tmp_value < 0) {
		*ptr++ = '-';
	}*/

	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}

	return result;
}

int 
strncmp(const char *a, const char *b, size_t n)
{
	char ca, cb;
	// memcp but with strings, since I know the type I won't do any pointer shenanigans	
	for (size_t i = 0; i < n; ++i) {
		ca = a[i], cb = b[i];
		if (!ca || !cb) {
			return 0;
		}
		if (ca != cb) {
			return ca < cb ? -1 : 1;
		}
	}

	return 0;
}

int 
strcmp(const char *a, const char *b)
{
	char ca, cb;
	for (size_t i = 0; ; ++i) {
		ca = a[i], cb = b[i];
		if (!ca || !cb) {
			return 0;
		}
		if (ca != cb) {
			return ca < cb ? -1 : 1;
		}
	}
}

int 
toupper(int c) {
	if (c >= 'a' && c <= 'z') {
		return c - 0x20;
	}
	return c;
}

