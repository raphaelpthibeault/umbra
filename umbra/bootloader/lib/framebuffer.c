#include "framebuffer.h"
#include <mm/pmm.h>
#include <drivers/edid.h>
#include <drivers/vbe.h>
#include <drivers/serial.h>

void
fb_clear(struct fb_info *fb)
{
	for (size_t y = 0; y < fb->framebuffer_height; y++) {
		switch (fb->framebuffer_bpp) {
			case 32: {
				uint32_t *fbp = (void *)(uintptr_t)fb->framebuffer_addr;
				size_t row = (y * fb->framebuffer_pitch) / 4;
				for (size_t x = 0; x < fb->framebuffer_width; x++) {
						fbp[row + x] = 0;
				}
				break;
			}
			case 16: {
				uint16_t *fbp = (void *)(uintptr_t)fb->framebuffer_addr;
				size_t row = (y * fb->framebuffer_pitch) / 2;
				for (size_t x = 0; x < fb->framebuffer_width; x++) {
						fbp[row + x] = 0;
				}
				break;
			}
			default: {
				uint8_t *fbp = (void *)(uintptr_t)fb->framebuffer_addr;
				size_t row = y * fb->framebuffer_pitch;
				for (size_t x = 0; x < fb->framebuffer_width * fb->framebuffer_bpp; x++) {
						fbp[row + x] = 0;
				}
				break;
			}
		}
	}
}

struct fb_info *
fb_init(size_t *_fb_count, uint16_t target_width, uint16_t target_height, uint16_t target_bpp)
{
	serial_print("Framebuffer: Initializing...\n");
	struct fb_info *ret = ext_mem_alloc(sizeof(struct fb_info));	

	if (vbe_init(ret, target_width, target_height, target_bpp)) {
		*_fb_count = 1;
		ret->edid = get_edid_record();
		size_t mode_count;
		ret->mode_list = vbe_get_mode_list(&mode_count);
		ret->mode_count = mode_count;
	} else {
		*_fb_count = 0;
		memmap_free(ret, sizeof(struct fb_info));
	}

	serial_print("Framebuffer: Initialized with dims %dx%dx%d\n", (uint16_t)ret->framebuffer_width, (uint16_t)ret->framebuffer_height, (uint16_t)ret->framebuffer_bpp);
	return ret;
}
