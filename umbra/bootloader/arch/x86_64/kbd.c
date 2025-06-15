#include "kbd.h"
#include <types.h>
#include <drivers/serial.h>

/* reference:
 * https://wiki.osdev.org/Programmable_Interval_Timer
 * */

/* for the graphical terminal, not for serial */

int 
getchar(void)
{
	while (true)
	{
		int ret = pit_sleep_and_quit_on_keypress(65535);
		serial_print("ret: %d\n", ret);
		if (ret != 0)
			return ret;
	}
}

int
getchar_internal(uint8_t scancode, uint8_t ascii, uint32_t shift_state)
{
	switch (scancode)
	{
		case 0x44:
			return GETCHAR_F10;
		case 0x4b:
			return GETCHAR_CURSOR_LEFT;
		case 0x4d:
			return GETCHAR_CURSOR_RIGHT;
		case 0x48:
			return GETCHAR_CURSOR_UP;
		case 0x50:
			return GETCHAR_CURSOR_DOWN;
		case 0x53:
			return GETCHAR_DELETE;
		case 0x4f:
			return GETCHAR_END;
		case 0x47:
			return GETCHAR_HOME;
		case 0x49:
			return GETCHAR_PGUP;
		case 0x51:
			return GETCHAR_PGDOWN;
		case 0x01:
			return GETCHAR_ESCAPE;
	}

	switch (ascii) 
	{
		case '\n':
		case '\r':
			return '\n';
		case '\b':
			return '\b';
		case '\t':
			return '\t';
	}

	if (shift_state & (GETCHAR_LCTRL | GETCHAR_RCTRL)) 
	{
		switch (ascii) 
		{
			case 'a': return GETCHAR_HOME;
			case 'e': return GETCHAR_END;
			case 'p': return GETCHAR_CURSOR_UP;
			case 'n': return GETCHAR_CURSOR_DOWN;
			case 'b': return GETCHAR_CURSOR_LEFT;
			case 'f': return GETCHAR_CURSOR_RIGHT;
			default: break;
		}
	}

	/* guard against non-printable values */
	if (ascii < 0x20 || ascii > 0x7e) 
			return -1;

	return ascii;
}

