#include "serial.h"
#include <types.h>
#include <lib/arg.h>
#include <lib/misc.h>

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

void 
serial_print_buffer_hex(void *buf, size_t size)
{
	if (buf == NULL) 
	{
		serial_print("(NULL buf)\n");
		return;
	}
	if (size == 0) 
	{
		serial_print("(empty buf)\n");
		return;
	}

	unsigned char *p = (unsigned char *)buf;
	char hex_representation[3];
	// process one char at a time 
	// max 16 hex digits for a 64-bit size_t, plus null terminator.
	char offset_str_buf[17];

	const size_t bytes_per_line = 16; 

	for (size_t i = 0; i < size; ++i) 
	{
		if (i % bytes_per_line == 0) 
		{
			if (i > 0) 
			{
				serial_print("\n");
			}

			itoa(i, offset_str_buf, 16);

			serial_print("%s", offset_str_buf);

			serial_print(":");
			serial_print(" ");
		}

		byte_to_hex_string(p[i], hex_representation);
		serial_print("%s", hex_representation);

		if (i < size - 1) {

			if ((i + 1) % bytes_per_line != 0) 
			{ 
				serial_print(" ");

				if ((bytes_per_line / 2 > 0) && ((i + 1) % (bytes_per_line / 2) == 0)) 
				{
					serial_print(" ");
				}
			}
		}
	}

	serial_print("\n");
}
