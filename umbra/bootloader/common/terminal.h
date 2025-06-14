#ifndef __TERMINAL_H__
#define __TERMINAL_H__

/* Umbra bootloader's boot menu graphical terminal */

#include <types.h>
#include <lib/video.h>

typedef struct {
	const char *name;
	const char *path;
} terminal_menu_entry_t;

typedef struct {
	int size;
	terminal_menu_entry_t *entry_list;
} terminal_menu_t;


struct fb_char {
	uint32_t c;
	uint32_t fg;
	uint32_t bg;
};

struct terminal_ctx {
	uint32_t *fb;	
	struct video_mode_info; // TODO

	size_t rows, cols;	
	size_t height, width;
	size_t pitch;
	size_t bytes_per_pixel;

	size_t offset_x;
	size_t offset_y;


	size_t grid_size;
	struct fb_char *grid;
	size_t font_height;
	size_t font_width;
	size_t glyph_width;
	size_t glyph_height;


	uint32_t bg_color;
	uint32_t fg_color;

	uint8_t red_mask_size;
	uint8_t red_mask_shift;
	uint8_t green_mask_size;
	uint8_t green_mask_shift;
	uint8_t blue_mask_size;
	uint8_t blue_mask_shift;

	bool cursor_enabled;
};




bool terminal_init(void);

#endif // !__TERMINAL_H__
