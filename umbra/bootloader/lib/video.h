#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <types.h>

typedef enum {
	VIDEO_MODE_TYPE_RGB = 0x00000001,
	VIDEO_MODE_TYPE_INDEX_COLOR = 0x00000002,
	VIDEO_MODE_TYPE_1BIT_BITMAP = 0x00000004,
	VIDEO_MODE_TYPE_YUV = 0x00000008,
	
	/* defines used to mask flags.  */
	VIDEO_MODE_TYPE_COLOR_MASK = 0x0000000F,
	
	VIDEO_MODE_TYPE_DOUBLE_BUFFERED = 0x00000010,
	VIDEO_MODE_TYPE_ALPHA = 0x00000020,
	VIDEO_MODE_TYPE_PURE_TEXT = 0x00000040,
	VIDEO_MODE_TYPE_UPDATING_SWAP = 0x00000080,
	VIDEO_MODE_TYPE_OPERATIONAL_MASK = 0x000000F0,
	
	/* defines used to specify requested bit depth.  */
	VIDEO_MODE_TYPE_DEPTH_MASK = 0x0000FF00,
#define VIDEO_MODE_TYPE_DEPTH_POS 8

	VIDEO_MODE_TYPE_UNKNOWN = 0x00010000,
	VIDEO_MODE_TYPE_HERCULES = 0x00020000,
	VIDEO_MODE_TYPE_PLANAR = 0x00040000,
	VIDEO_MODE_TYPE_NONCHAIN4 = 0x00080000,
	VIDEO_MODE_TYPE_CGA = 0x00100000,
	VIDEO_MODE_TYPE_INFO_MASK = 0x00FF0000,
} video_mode_type_t;

struct video_mode_info {
	unsigned int width;
	unsigned int height;
	video_mode_type_t mode_type; /* Mode type bitmask, has information like is it Index color or RGB mode.  */
	unsigned int bpp; /* Bits per pixel.  */
	unsigned int bytes_per_pixel; /* Bytes per pixel.  */
	unsigned int pitch; /* Pitch of one scanline.  How many bytes there are for scanline.  */
	unsigned int number_of_colors; /* In index color mode, number of colors.  In RGB mode this is 256.  */
	unsigned int mode_number;
#define VIDEO_MODE_NUMBER_INVALID 0xffffffff

	unsigned int red_mask_size; 
	unsigned int red_field_pos;
	unsigned int green_mask_size;
	unsigned int green_field_pos;
	unsigned int blue_mask_size;
	unsigned int blue_field_pos;
	unsigned int reserved_mask_size;
	unsigned int reserved_field_pos;
	
	/* For 1-bit bitmaps, the background color.  Used for bits = 0.  */
	uint8_t bg_red;
	uint8_t bg_green;
	uint8_t bg_blue;
	uint8_t bg_alpha;
	
	/* For 1-bit bitmaps, the foreground color.  Used for bits = 1.  */
	uint8_t fg_red;
	uint8_t fg_green;
	uint8_t fg_blue;
	uint8_t fg_alpha;	
};


#endif // !__VIDEO_H__
