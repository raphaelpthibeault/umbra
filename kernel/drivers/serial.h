#ifndef __DRIVERS_SERIAL_H___
#define __DRIVERS_SERIAL_H___

#include <types.h>

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, type) __builtin_va_arg(v, type)

int serial_print(const char *fmt, ...);
void serial_print_buffer_hex(void *buf, size_t size);

#endif // !__DRIVERS_SERIAL_H___
