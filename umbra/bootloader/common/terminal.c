#include "terminal.h"
#include <types.h>

#include <drivers/serial.h>
#include <lib/framebuffer.h>
#include <mm/pmm.h>

bool
terminal_init()
{
	struct fb_info *fb = NULL; 
	size_t fb_count = 0;;

	serial_print("\n\n\n---------- UMBRA BOOTLOADER ----------\n\n");

	serial_print("Terminal: Initializing...\n");
	//fb = fb_init(&fb_count, 640, 480, 32);
	fb = fb_init(&fb_count, 0, 0, 32);

	if (fb == NULL || fb_count == 0) {
		serial_print("Terminal: Error, framebuffer null\n");
		return false;
	}

	serial_print("Terminal: initialized with dims %dx%dx%d\n", (uint16_t)fb->framebuffer_width, (uint16_t)fb->framebuffer_height, (uint16_t)fb->framebuffer_bpp);

	/* just draws a white line 
	*/
	for (size_t i = 0; i < 100; i++) {
		volatile uint32_t *fb_ptr = (uint32_t*)fb->framebuffer_addr;
		fb_ptr[i] = 0xffffff;
	}

	return true;
}

