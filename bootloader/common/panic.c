#include <common/panic.h>
#include <drivers/serial.h>
#include <types.h>
#include <lib/arg.h>
#include <lib/misc.h>

/* TODO: a param for which output to use?
 * enum 
 * 0 = serial
 * 1 = vga
 * 2 = framebuffer
 * */
noreturn void 
panic(const char *fmt, ...)
{
	va_list args;
	char buf[256], *s = (char *)&buf;

	va_start(args, fmt);
	vprint(buf, fmt, args);
	va_end(args);

	serial_print(" --- PANIC --- ");
	while (*s) {
		if (*s == '\n') {
			serial_out('\r');
		}

		serial_out(*s++);	
	}
	serial_out('\r');
	serial_out('\n');

	/* TODO print stacktrace */

	while (1);

	__builtin_unreachable();
}

