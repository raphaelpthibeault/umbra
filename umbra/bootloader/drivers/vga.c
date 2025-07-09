#include <drivers/vga.h>
#include <types.h>
#include <lib/misc.h>

#include <arch/x86_64/cpu.h>

volatile vga_char_t *TEXT_AREA = (vga_char_t*) VGA_START;

static uint8_t 
vga_color(const uint8_t fg, const uint8_t bg) 
{
	// place background colour in the higher 4 bits and mask the foreground bits
	return (bg << 4) | (fg & 0x0F);
}

void 
clearwin(const uint8_t fg, const uint8_t bg) 
{
	const char space = ' ';
	uint8_t clear_color = vga_color(fg, bg);

	vga_char_t clear_char = {
		.c = space,
		.style = clear_color
	};

	for (uint64_t i = 0; i < VGA_SIZE; ++i)
		TEXT_AREA[i] = clear_char;
}

static void 
putchar(const char c, const uint8_t fg, const uint8_t bg) 
{
	uint16_t pos = get_cursor_pos();

	// handle special characters
	if (c == '\n') {
		uint8_t curr_row = (uint8_t)(pos / VGA_WIDTH);

		if (++curr_row >= VGA_HEIGHT)
				scroll_line();
		else 
				set_cursor_pos(0, curr_row);
			
	} else if (c == '\b') {
		reverse_cursor();
		putchar(' ', fg, bg);
		reverse_cursor();
	} else if (c == '\r') {
		uint8_t curr_row = (uint8_t)(pos / VGA_WIDTH);
		set_cursor_pos(0, curr_row);
	} else if (c == '\t') {
		// conventionally, 1 tab = 4 spaces
		for (uint8_t i = 0; i < 4; ++i) 
			putchar(' ', fg, bg);
		advance_cursor();
	} else {
		uint8_t style = vga_color(fg, bg);

		vga_char_t tmp = {
			.c = c,
			.style = style
		};

		TEXT_AREA[pos] = tmp;

		advance_cursor();
	}
}

void 
putstr(const char *str, const uint8_t fg, const uint8_t bg) 
{
	while (*str != '\0') {
		putchar(*str++, fg, bg);
		advance_cursor();
	}
}

uint16_t 
get_cursor_pos() 
{
	uint16_t pos = 0;

	outb(CURSOR_PORT_COMMAND, 0x0F);
	pos |= inb(CURSOR_PORT_DATA);

	outb(CURSOR_PORT_COMMAND, 0x0E);
	pos |= inb(CURSOR_PORT_DATA) << 8;

	return pos;
}

void 
show_cursor() 
{
	uint8_t curr;
	
	outb(CURSOR_PORT_COMMAND, 0x0A);
	curr = inb(CURSOR_PORT_DATA);
	outb(CURSOR_PORT_DATA, curr & 0xC0);

	outb(CURSOR_PORT_COMMAND, 0x0B);
	curr = inb(CURSOR_PORT_DATA);
	outb(CURSOR_PORT_DATA, curr & 0xE0);
}

void 
hide_cursor() 
{
	outb(CURSOR_PORT_COMMAND, 0x0A);
	outb(CURSOR_PORT_DATA, 0x20);
}

void 
advance_cursor() 
{
	uint16_t pos = get_cursor_pos();
	++pos;

	if (pos >= VGA_SIZE)
		scroll_line();

	outb(CURSOR_PORT_COMMAND, 0x0F);
	outb(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));

	outb(CURSOR_PORT_COMMAND, 0x0E);
	outb(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void 
reverse_cursor() 
{
	uint16_t pos = get_cursor_pos();
	--pos;

	if (pos >= VGA_SIZE)
		pos = VGA_SIZE -1;

	outb(CURSOR_PORT_COMMAND, 0x0F);
	outb(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));

	outb(CURSOR_PORT_COMMAND, 0x0E);
	outb(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void 
set_cursor_pos(uint8_t x, uint8_t y) 
{
	uint16_t pos = x + ((uint16_t)VGA_WIDTH * y);

	if (pos >= VGA_SIZE)
		pos = VGA_SIZE -1;

	outb(CURSOR_PORT_COMMAND, 0x0F);
	outb(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));

	outb(CURSOR_PORT_COMMAND, 0x0E);
	outb(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void 
scroll_line() 
{
	// move text up in VGA memory by copying it up (obviously will lose topmost row)
	uint16_t i,j;
	for (i = 1; i < VGA_HEIGHT; ++i) {
		for (j = 0; j < VGA_WIDTH; ++j) {
			uint16_t src = j + (i * VGA_WIDTH);
			uint16_t dest = j + ((i-1) * VGA_WIDTH);

			TEXT_AREA[dest] = TEXT_AREA[src];
		}
	}
	
	// clear bottommost row
	i = VGA_HEIGHT - 1;

	for (j = 0; j < VGA_WIDTH; ++j) {
		uint16_t pos = j + (i * VGA_WIDTH);

		vga_char_t curr = TEXT_AREA[pos];
		vga_char_t clear = {
			.c = ' ',
			.style = curr.style
		};
		
		TEXT_AREA[pos] = clear;
	}
	
	set_cursor_pos(0, VGA_HEIGHT-1);
}

void 
print_buffer_hex(void *buf, size_t size, const uint8_t fg, const uint8_t bg)
{
	if (buf == NULL) {
		putstr("(NULL buf)\n", fg, bg);
		return;
	}
	if (size == 0) {
			putstr("(empty buf)\n", fg, bg);
			return;
	}

	unsigned char *p = (unsigned char *)buf;
	char hex_representation[3];
	// process one char at a time 
	char c[2];

	// max 16 hex digits for a 64-bit size_t, plus null terminator.
	char offset_str_buf[17];

	const size_t bytes_per_line = 16; 

	for (size_t i = 0; i < size; ++i) {
		if (i % bytes_per_line == 0) {
			if (i > 0) { 
				c[0] = '\n';
				putstr(c, fg, bg);
			}

			itoa(i, offset_str_buf, 16);
			putstr(offset_str_buf, fg, bg);

			c[0] = ':';
			putstr(c, fg, bg);
			c[0] = ' ';
			putstr(c, fg, bg);
		}

		byte_to_hex_string(p[i], hex_representation);
		putstr(hex_representation, fg, bg);

		if (i < size - 1) { 
			if ((i + 1) % bytes_per_line != 0) { 
				c[0] = ' ';
				putstr(c, fg, bg);

				if ((bytes_per_line / 2 > 0) && ((i + 1) % (bytes_per_line / 2) == 0)) {
					putstr(c, fg, bg);
				}
			}
		}
	}

	c[0] = '\n';
	putstr(c, fg, bg);
}

