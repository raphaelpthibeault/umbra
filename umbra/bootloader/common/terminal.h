#ifndef __TERMINAL_H__
#define __TERMINAL_H__

/* Umbra bootloader's boot menu graphical terminal */

#include <types.h>
#include <lib/video.h>
#include <lib/arg.h>

/* determines whether or not a newline is carriage return */
#define OOB_OUTPUT_ONLCR (1 << 4)

struct fb_char {
	uint32_t c;
	uint32_t fg;
	uint32_t bg;
};

struct fb_queue_item {
	size_t x, y;
	struct fb_char c;	
};

struct terminal_ctx {
	uint32_t *fb;	

	size_t rows, cols;	
	size_t height, width;
	size_t pitch;
	size_t bytes_per_pixel;

	size_t offset_x;
	size_t offset_y;

	size_t cursor_x, old_cursor_x;
	size_t cursor_y, old_cursor_y;

	size_t scroll_top_margin;
	size_t scroll_bottom_margin;

	size_t grid_size;
	struct fb_char *grid;

	size_t queue_i;
	size_t queue_size;
	struct fb_queue_item *queue;
	size_t map_size;
	struct fb_queue_item **map;

	size_t font_height;
	size_t font_width;
	size_t font_bool_size;
	bool *font_bool;
	size_t font_bits_size;
	uint8_t *font_bits;

	size_t glyph_width;
	size_t glyph_height;

	uint32_t bg_color, old_bg_color;
	uint32_t fg_color, old_fg_color;

	uint8_t red_mask_size;
	uint8_t red_mask_shift;
	uint8_t green_mask_size;
	uint8_t green_mask_shift;
	uint8_t blue_mask_size;
	uint8_t blue_mask_shift;

	bool cursor_enabled;
	bool scroll_enabled;

	uint32_t ansi_colors[8];
	uint32_t ansi_bright_colors[8];

	size_t oob_output;
};

bool terminal_init(void);
void terminal_write(const char *msg);
int terminal_print(const char *fmt, ...);
void terminal_get_cursor_pos(size_t *x, size_t *y);
void terminal_set_cursor_pos(size_t x, size_t y);
void terminal_set_color(uint32_t fg, uint32_t bg);
void terminal_clear(void);
void terminal_disable_cursor(void);
void terminal_enable_cursor(void);

extern struct terminal_ctx *term_ctx;

#endif // !__TERMINAL_H__
