#ifndef __DRIVER_VGA_H__
#define __DRIVER_VGA_H__

#include <types.h>

#define VGA_START 0xB8000
#define VGA_SIZE 80*25
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define CURSOR_PORT_COMMAND (uint16_t) 0x3D4
#define CURSOR_PORT_DATA    (uint16_t) 0x3D5

#define COLOR_BLK 0     /* Black */
#define COLOR_BLU 1     /* Blue */
#define COLOR_GRN 2     /* Green */
#define COLOR_CYN 3     /* Cyan */
#define COLOR_RED 4     /* Red */
#define COLOR_PRP 5     /* Purple */
#define COLOR_BRN 6     /* Brown */
#define COLOR_GRY 7     /* Gray */
#define COLOR_DGY 8     /* Dark Gray */
#define COLOR_LBU 9     /* Light Blue */
#define COLOR_LGR 10    /* Light Green */
#define COLOR_LCY 11    /* Light Cyan */
#define COLOR_LRD 12    /* Light Red */
#define COLOR_LPP 13    /* Light Purple */
#define COLOR_YEL 14    /* Yellow */
#define COLOR_WHT 15    /* White */

typedef struct {
    char c;
    char style;
} __attribute__((packed)) vga_char_t;

/* get char to use as style char */
uint8_t vga_color(const uint8_t fg, const uint8_t bg);
/* clear window */
void clearwin(const uint8_t fg, const uint8_t bg);
/* put char on screen */
void putchar(const char c, const uint8_t fg, const uint8_t bg);
/* put string on screen */
void putstr(const char *str, const uint8_t fg, const uint8_t bg);

/* get cursor position */
uint16_t get_cursor_pos();
/* show cursor */ 
void show_cursor();
/* hide crusor */ 
void hide_cursor();
/* advance cursor rightwards */
void advance_cursor();
/* reverse cursor leftwards */
void reverse_cursor();
/* set cursor position */
void set_cursor_pos(uint8_t x, uint8_t y);
/* scroll line i.e move cursor down and place leftmost */
void scroll_line();

#endif /* ifndef __DRIVER_VGA_H__ */
