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

size_t 
strlen(const char *str)
{
	size_t i;

	for (i = 0; str[i]; ++i);

	return i;
}

char *
strcpy(char *dest, const char *src)
{
	size_t i;
	for (i = 0; src[i]; ++i) {
		dest[i] = src[i];
	}

	dest[i] = 0;

	return dest;	
}

char *
strchr(const char *s, int c)
{
	do {
		if (*s == c) {
			return (char *)s;
		}
	}	while (*s++);
	return 0;
}

int
digit_to_int(char c)
{
	if (c >= 'a' && c <= 'f') {
		return (c - 'a') + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return (c - 'A') + 10;
	}
	if (c >= '0' && c <= '9'){
		return c - '0';
	}

	return -1;
}

uint64_t
strtoui(const char *s, const char **end, int base)
{
	uint64_t n = 0;
	for (size_t i = 0; ; ++i) {
		int d = digit_to_int(s[i]);
		if (d == -1) {
			if (end != NULL) {
				*end = &s[i];
			}
			break;
		}
		n = n * base + d;
	}

	return n;
}

