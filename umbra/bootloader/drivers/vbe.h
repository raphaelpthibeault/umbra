#ifndef __VBE_H__
#define __VBE_H__

#include <types.h>
#include <lib/framebuffer.h>

bool vbe_init(struct fb_info *ret, uint16_t target_width, uint16_t target_height, uint16_t target_bpp);
struct fb_info *vbe_get_mode_list(size_t *count);

#endif // !__VBE_H__
