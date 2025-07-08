#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <types.h>
#include <lib/arg.h>

void serial_out(uint8_t b);
int serial_print(const char *fmt, ...);

void serial_print_buffer_hex(void *buf, size_t size);
#endif // !__SERIAL_H__
