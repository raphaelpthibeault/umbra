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


static uint8_t hex_lookup[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

void
byte_to_hex_string(uint8_t byte, char *out)
{
	out[0] = hex_lookup[(byte>>4)&0xf];
	out[1] = hex_lookup[byte&0xf];
	out[2] = '\0';
}

/*  reference: https://cplusplus.com/reference/cstdio/printf/
 *  %[flags][width][.precision][length]specifier  
 *  flags:
 *		%% prints a '%' to stream 
 *		0 left pads with 0 instead of ' '
 *		- not supported
 *		+ not supported
 *		' ' not supported
 *		# not supported
 *	width:
 *	precision: not supported
 *	length: not supported
 *
 * */
/* very minimal vsprintf */
size_t 
vprint(char *dst, const char *fmt, va_list args) 
{
	char *orig = dst, pad = ' ', tmpstr[22], *p;
	int pad_len, sign, i;

	if (dst == (void *)0 || fmt == (void *)0) {
		return 0;
	}

	while (*fmt) {
		if (*fmt == '%') { /* format specifier */
			++fmt;
			/* flags */
			// "%%"
			if (*fmt == '%') goto putc;
			if (*fmt == '0') pad = '0';
			/* width */
			pad_len = 0;
			while (*fmt >= '0'&& *fmt <= '9') {
				pad_len *= 10;	
				pad_len += *fmt - '0';
				++fmt;
			}
			/* specifier */
			if (*fmt == 'l') { // ignore
				++fmt;
			}
			if (*fmt == 'c') {
				int arg = va_arg(args, int);
				*dst++ = (char)arg;
				++fmt;
				continue;
			} else if (*fmt == 'd') {
				int arg = va_arg(args, int);
				sign = 0;
				if (arg < 0) {
					arg *= -1;
					++sign;
				}

				i = 21;
				tmpstr[i] = 0;
				do {
					tmpstr[--i] = '0' + (arg % 10);
					arg /= 10;
				} while (arg != 0 && i > 0);
				if (sign) {
					tmpstr[--i] = '-';
				}
				/* time to put, check padding first */
				if (pad_len > 0 && pad_len < 21) {
					while (i > 21 - pad_len) {
						tmpstr[--i] = pad;
					}
				}
				p = &tmpstr[i];
				goto copystr;
			} else if (*fmt == 'x') {
				/* fragile and broken, I know */
				uint32_t arg = va_arg(args, unsigned long long);	
				i = 21;
				tmpstr[i] = 0;
				do {
					/* 0-9 -> '0'-'9', 10-15 -> 'a'-'f'*/	
					char n = arg & 0xf;
					tmpstr[--i] = n + (n > 9 ? 0x57 : 0x30); // 0x61 - 10 = 0x57
					arg >>= 4;
				} while (arg != 0 && i > 0);

				/* time to put, check padding first */
				if (pad_len > 0 && pad_len <= 21) { // no negatives, so <=
					while (i > 21 - pad_len) {
						tmpstr[--i] = '0'; // since it's hex, I'll only do leading zeroes so printing to-length is easy
					}
				}
				p = &tmpstr[i];
				goto copystr;
			} else if (*fmt == 's') {
				p = va_arg(args, char *);
copystr:
				if (p == (void *)0) {
					p = "(null)";		
				}
				while (*p) {
					*dst++ = *p++;
				}
			}
		}	else {
putc:	
			*dst++ = *fmt;
		}

		++fmt;
	}

	*dst = 0;
	return dst - orig;
}

