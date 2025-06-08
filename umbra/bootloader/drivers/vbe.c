#include "vbe.h"
#include <types.h>
#include <lib/framebuffer.h>
#include <drivers/serial.h>
#include <lib/misc.h>
#include <mm/pmm.h>

#include <arch/x86_64/real.h>

/* reference: https://wiki.osdev.org/VESA_Video_Modes */

struct vbe_info_block {
	char signature[4];
	uint8_t version_lo;
	uint8_t version_hi;
	uint16_t oem_off;
	uint16_t oem_seg;
	uint32_t capabilities;
	uint16_t vid_modes_off;
	uint16_t vid_modes_seg;
	uint16_t total_memory;
	uint16_t software_revision;
	uint16_t vendor_off;
	uint16_t vendor_seg;
	uint16_t prod_name_off;
	uint16_t prod_name_seg;
	uint16_t prod_rev_off;
	uint16_t prod_rev_seg;
	uint8_t  reserved[222];
	uint8_t  oem_data[256];
} __attribute__((packed));

struct vbe_mode_info_structure  {
	uint16_t mode_attributes;
	uint8_t  wina_attributes;
	uint8_t  winb_attributes;
	uint16_t win_granularity;
	uint16_t win_size;
	uint16_t wina_segment;
	uint16_t winb_segment;
	uint32_t win_farptr;
	uint16_t bytes_per_scanline;

	uint16_t res_x;
	uint16_t res_y;
	uint8_t  charsize_x;
	uint8_t  charsize_y;
	uint8_t  plane_count;
	uint8_t  bpp;
	uint8_t  bank_count;
	uint8_t  memory_model;
	uint8_t  bank_size;
	uint8_t  image_count;
	uint8_t  reserved0;

	uint8_t  red_mask_size;
	uint8_t  red_mask_shift;
	uint8_t  green_mask_size;
	uint8_t  green_mask_shift;
	uint8_t  blue_mask_size;
	uint8_t  blue_mask_shift;
	uint8_t  rsvd_mask_size;
	uint8_t  rsvd_mask_shift;
	uint8_t  direct_color_info;

	uint32_t framebuffer_addr;
	uint8_t  reserved1[6];

	uint16_t lin_bytes_per_scanline;
	uint8_t  banked_image_count;
	uint8_t  lin_image_count;
	uint8_t  lin_red_mask_size;
	uint8_t  lin_red_mask_shift;
	uint8_t  lin_green_mask_size;
	uint8_t  lin_green_mask_shift;
	uint8_t  lin_blue_mask_size;
	uint8_t  lin_blue_mask_shift;
	uint8_t  lin_rsvd_mask_size;
	uint8_t  lin_rsvd_mask_shift;
	uint32_t max_pixel_clock;

	uint8_t  reserved2[190];
} __attribute__((packed));

static int
vbe_get_controller_info(struct vbe_info_block *ci)
{
	struct int_regs regs = {0};
	regs.es = (((uintptr_t)ci) & 0xffff0000) >> 4;
  regs.edi = ((uintptr_t)ci) & 0xffff;
  regs.eax = 0x4f00;
  regs.flags = 0x200;

  rm_int(0x10, &regs);

  return regs.eax & 0xffff;
}

static int
vbe_get_mode_info(struct vbe_mode_info_structure *mode_info, uint32_t mode)
{
	struct int_regs regs = {0};
	regs.es = (((uintptr_t)mode_info) & 0xffff0000) >> 4;
  regs.edi = ((uintptr_t)mode_info) & 0xffff;
  regs.eax = 0x4f01;
	regs.ecx = mode;
  regs.flags = 0x200;

  rm_int(0x10, &regs);

  return regs.eax & 0xffff;
}

bool
vbe_init(struct fb_info *ret, uint16_t target_width, uint16_t target_height, uint16_t target_bpp)
{
	serial_print("VBE: Initializing...\n");

	size_t curr_fallback = 0;

	struct vbe_info_block *vbe_info = (struct vbe_info_block *)SCRATCH_ADDR;
	memset(vbe_info, 0, sizeof(struct vbe_info_block));
	if (vbe_get_controller_info(vbe_info) != 0x4f) {
		return false;
	}

	serial_print("VBE: Version: %d.%d\n", vbe_info->version_hi, vbe_info->version_lo);
	serial_print("VBE: OEM: %s\n", (char *)rm_desegment(vbe_info->oem_seg, vbe_info->oem_off));
	serial_print("VBE: Graphics vendor: %s\n", (char *)rm_desegment(vbe_info->vendor_seg, vbe_info->vendor_off));
	serial_print("VBE: Product name: %s\n", (char *)rm_desegment(vbe_info->prod_name_seg, vbe_info->prod_name_off));
	serial_print("VBE: Product revision: %s\n", (char *)rm_desegment(vbe_info->prod_rev_seg, vbe_info->prod_rev_off));

	uint32_t *vid_modes = (uint32_t *)rm_desegment(vbe_info->vid_modes_seg, vbe_info->vid_modes_off);
	
	struct resolution fallback_resolutions[] = {
		{ 1024, 768, 32 },
		{ 800,  600, 32 },
		{ 640,  480, 32 },
		{ 1024, 768, 24 },
		{ 800,  600, 24 },
		{ 640,  480, 24 },
		{ 1024, 768, 16 },
		{ 800,  600, 16 },
		{ 640,  480, 16 }
	};

	if (!target_width || !target_height || !target_bpp) {
		struct edid_record *edid_record = get_edid_record();	
		if (edid_record != NULL) {
			int edid_width = edid_record->detailed_timings[0].horizontal_active_lo + ((edid_record->detailed_timings[0].horizontal_hi & 0xf0) << 4);
			int edid_height = edid_record->detailed_timings[0].vertical_active_lo + ((edid_record->detailed_timings[0].vertical_hi & 0xf0) << 4);

			if (edid_width && edid_height) {
				target_width = edid_width;
				target_height = edid_height;
				target_bpp = 32;
				serial_print("VBE: EDID detected screen resolution of %dx%d\n", target_width, target_height);
				goto retry;
			}
		}
		goto fallback;
	} else {
		serial_print("VBE: Requested resolution of %dx%dx%d\n", target_width, target_height, target_bpp);
	}

retry:
	serial_print("VBE: Try with %dx%dx%d\n", target_width, target_height, target_bpp);
	for (size_t i = 0; vid_modes[i] != 0xffff; ++i) {
		struct vbe_mode_info_structure mode_info = {0};

		if (vbe_get_mode_info(&mode_info, vid_modes[i]) != 0x004f) {
			continue;	
		}

		if (mode_info.res_x == target_width &&
				mode_info.res_y == target_height &&
				mode_info.bpp == target_bpp) {
			/* I'll only support RGB probably */
			if (mode_info.memory_model != 0x06) {
				continue;
			}

			/* only support linear modes */
			if (!(mode_info.mode_attributes & (1 << 7))) {
				continue;
			}

			serial_print("VBE: Found matching mode 0x%x, try to set...\n", vid_modes[i]);

			serial_print("VBE: Framebuffer address: 0x%x\n", mode_info.framebuffer_addr);

			ret->memory_model = mode_info.memory_model;
			ret->framebuffer_addr = mode_info.framebuffer_addr;
			ret->framebuffer_width = mode_info.res_x;
			ret->framebuffer_height = mode_info.res_y;
			ret->framebuffer_bpp = mode_info.bpp;

			if (vbe_info->version_hi < 3) {
				ret->framebuffer_pitch = mode_info.bytes_per_scanline;
				ret->red_mask_size = mode_info.red_mask_size;
				ret->red_mask_shift = mode_info.red_mask_shift;
				ret->green_mask_size = mode_info.green_mask_size;
				ret->green_mask_shift = mode_info.green_mask_shift;
				ret->blue_mask_size = mode_info.blue_mask_size;
				ret->blue_mask_shift = mode_info.blue_mask_shift;
			} else {
				ret->framebuffer_pitch = mode_info.lin_bytes_per_scanline;
				ret->red_mask_size = mode_info.lin_red_mask_size;
				ret->red_mask_shift = mode_info.lin_red_mask_shift;
				ret->green_mask_size = mode_info.lin_green_mask_size;
				ret->green_mask_shift = mode_info.lin_green_mask_shift;
				ret->blue_mask_size = mode_info.lin_blue_mask_size;
				ret->blue_mask_shift = mode_info.lin_blue_mask_shift;
			}

			fb_clear(ret);

			return true;
		}
	}

fallback:
	if (curr_fallback < SIZEOF_ARRAY(fallback_resolutions)) {
		target_width = fallback_resolutions[curr_fallback].width;
		target_height = fallback_resolutions[curr_fallback].height;
		target_bpp = fallback_resolutions[curr_fallback].bpp;
		++curr_fallback;
		goto retry;		
	}

	return false;
}

struct fb_info *
vbe_get_mode_list(size_t *count)
{
	struct vbe_info_block *vbe_info = (struct vbe_info_block *)SCRATCH_ADDR;
	memset(vbe_info, 0, sizeof(struct vbe_info_block));
	if (vbe_get_controller_info(vbe_info) != 0x4f) {
		return NULL;
	}

	uint32_t *vid_modes = (uint32_t *)rm_desegment(vbe_info->vid_modes_seg, vbe_info->vid_modes_off);
	size_t mode_count = 0;

	for (size_t i = 0; vid_modes[i] != 0xffff; ++i) {
		struct vbe_mode_info_structure mode_info = {0};

		if (vbe_get_mode_info(&mode_info, vid_modes[i]) != 0x004f) {
			continue;	
		}

		/* I'll only support RGB probably */
		if (mode_info.memory_model != 0x06) {
			continue;
		}

		/* only support linear modes */
		if (!(mode_info.mode_attributes & (1 << 7))) {
			continue;
		}

		++mode_count;
	}

	struct fb_info *ret = ext_mem_alloc(mode_count * sizeof(struct fb_info));

	for (size_t i = 0, j = 0; vid_modes[i] != 0xffff; ++i) {
		struct vbe_mode_info_structure mode_info = {0};

		if (vbe_get_mode_info(&mode_info, vid_modes[i]) != 0x004f) {
			continue;	
		}

		/* I'll only support RGB probably */
		if (mode_info.memory_model != 0x06) {
			continue;
		}

		/* only support linear modes */
		if (!(mode_info.mode_attributes & (1 << 7))) {
			continue;
		}

		ret[j].memory_model = mode_info.memory_model;
		ret[j].framebuffer_addr = mode_info.framebuffer_addr;
		ret[j].framebuffer_width = mode_info.res_x;
		ret[j].framebuffer_height = mode_info.res_y;
		ret[j].framebuffer_bpp = mode_info.bpp;

		if (vbe_info->version_hi < 3) {
			ret[j].framebuffer_pitch = mode_info.bytes_per_scanline;
			ret[j].red_mask_size = mode_info.red_mask_size;
			ret[j].red_mask_shift = mode_info.red_mask_shift;
			ret[j].green_mask_size = mode_info.green_mask_size;
			ret[j].green_mask_shift = mode_info.green_mask_shift;
			ret[j].blue_mask_size = mode_info.blue_mask_size;
			ret[j].blue_mask_shift = mode_info.blue_mask_shift;
		} else {
			ret[j].framebuffer_pitch = mode_info.lin_bytes_per_scanline;
			ret[j].red_mask_size = mode_info.lin_red_mask_size;
			ret[j].red_mask_shift = mode_info.lin_red_mask_shift;
			ret[j].green_mask_size = mode_info.lin_green_mask_size;
			ret[j].green_mask_shift = mode_info.lin_green_mask_shift;
			ret[j].blue_mask_size = mode_info.lin_blue_mask_size;
			ret[j].blue_mask_shift = mode_info.lin_blue_mask_shift;
		}

		++j;
	}

	*count = mode_count;
	return ret;
}

