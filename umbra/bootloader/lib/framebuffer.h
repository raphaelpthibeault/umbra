#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <types.h>
#include <drivers/edid.h>

struct resolution {
	uint64_t width;
	uint64_t height;
	uint16_t bpp;
};

struct fb_info {
	uint64_t framebuffer_pitch;
	uint64_t framebuffer_width;
	uint64_t framebuffer_height;
	uint16_t framebuffer_bpp;
	uint8_t memory_model;
	uint8_t red_mask_size;
	uint8_t red_mask_shift;
	uint8_t green_mask_size;
	uint8_t green_mask_shift;
	uint8_t blue_mask_size;
	uint8_t blue_mask_shift;

	uint64_t framebuffer_addr;

	struct edid_record *edid;

	//uint64_t mode_count;
	//struct fb_info *mode_list;
};

void fb_clear(struct fb_info *fb);
struct fb_info * fb_init(size_t *_fb_count, uint16_t target_width, uint16_t target_height, uint16_t target_bpp);

#endif // !__FRAMEBUFFER_H__
