#include "serial.h"
#include <types.h>
#include <lib/arg.h>

#include <arch/x86_64/cpu.h>

#include "vga.h"


#define PORT 0x3f8 /* COM1 */

static bool serial_initd = false;

static inline uint8_t 
is_transmit_empty() 
{
	return inb(PORT + 5) & 0x20;
}

static inline void
serial_init() 
{
	if (serial_initd) {
		return;
	}

	outb(PORT + 3, 0x00);
	outb(PORT + 1, 0x00); 
	outb(PORT + 3, 0x80);
											 
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);

	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xc7);
	outb(PORT + 4, 0x0b);

	serial_initd = true;
}

static size_t vprint(char *buf, const char *fmt, va_list args);

void 
serial_out(uint8_t b)
{
	serial_init();	
	while (is_transmit_empty() == 0);
	outb(PORT, b);
}

int
serial_print(const char *fmt, ...)
{
	va_list args;
	int ret;
	char buf[256], *s = (char *)&buf;

	va_start(args, fmt);
	ret = vprint(buf, fmt, args);
	va_end(args);

	while (*s) {
		if (*s == '\n') {
			serial_out('\r');
		}

		serial_out(*s++);	
	}

	return ret;
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
static size_t 
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
