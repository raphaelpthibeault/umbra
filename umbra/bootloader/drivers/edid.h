#ifndef __EDID_H__
#define __EDID_H__

#include <types.h>

/* EDID is the programmatical way to check if a video mode is supported */
/* reference: https://wiki.osdev.org/EDID */ 

struct edid_record
{
  uint8_t header[8];
  uint16_t manufacturer_id;
  uint16_t product_id;
  uint32_t serial_number;
  uint8_t week_of_manufacture;
  uint8_t year_of_manufacture;
  uint8_t version;
  uint8_t revision;

  uint8_t video_input_definition;
  uint8_t max_horizontal_image_size;
  uint8_t max_vertical_image_size;
  uint8_t display_gamma;
  uint8_t feature_support;
#define EDID_FEATURE_PREFERRED_TIMING_MODE (1 << 1)

  uint8_t red_green_lo;
  uint8_t blue_white_lo;
  uint8_t red_x_hi;
  uint8_t red_y_hi;
  uint8_t green_x_hi;
  uint8_t green_y_hi;
  uint8_t blue_x_hi;
  uint8_t blue_y_hi;
  uint8_t white_x_hi;
  uint8_t white_y_hi;

  uint8_t established_timings_1;
  uint8_t established_timings_2;
  uint8_t manufacturer_reserved_timings;

  uint16_t standard_timings[8];

  struct {
    uint16_t pixel_clock;
    /* Only valid if the pixel clock is non-null.  */
    uint8_t horizontal_active_lo;
    uint8_t horizontal_blanking_lo;
    uint8_t horizontal_hi;
    uint8_t vertical_active_lo;
    uint8_t vertical_blanking_lo;
    uint8_t vertical_hi;
    uint8_t horizontal_sync_offset_lo;
    uint8_t horizontal_sync_pulse_width_lo;
    uint8_t vertical_sync_lo;
    uint8_t sync_hi;
    uint8_t horizontal_image_size_lo;
    uint8_t vertical_image_size_lo;
    uint8_t image_size_hi;
    uint8_t horizontal_border;
    uint8_t vertical_border;
    uint8_t flags;
  } detailed_timings[4];

  uint8_t extension_flag;
  uint8_t checksum;
} __attribute__((packed));

struct edid_record *get_edid_record(void);

#endif // !__EDID_H__
