#include "vbe.h"
#include <types.h>
#include <drivers/serial.h>
#include <lib/framebuffer.h>
#include <lib/misc.h>
#include <lib/video.h>
#include <mm/pmm.h>

#include <arch/x86_64/real.h>

/* reference: https://wiki.osdev.org/VESA_Video_Modes */

#define VBE_MODEATTR_SUPPORTED (1 << 0)
#define VBE_MODEATTR_COLOR (1 << 3)
#define VBE_MODEATTR_LFB_AVAIL (1 << 7)
#define VBE_MODEATTR_GRAPHICS (1 << 4)
#define VBE_MEMORY_MODEL_PACKED_PIXEL 0x04
#define VBE_MEMORY_MODEL_DIRECT_COLOR 0x06

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

struct vbe_mode_info_block  {
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

struct vbe_crtc_info_block {
	uint16_t horizontal_total;
	uint16_t horizontal_sync_start;
	uint16_t horizontal_sync_end;
	uint16_t vertical_total;
	uint16_t vertical_sync_start;
	uint16_t vertical_sync_end;
	uint8_t flags;
	uint32_t pixel_clock;
	uint16_t refresh_rate;
	uint8_t reserved[40];	
} __attribute__((packed));

static struct vbe_info_block controller_info;
static int vbe_detected = -1;
static uint16_t *vbe_mode_list;
static uint32_t initial_vbe_mode;

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

bool
vbe_probe(struct vbe_info_block *info_block)
{
	struct vbe_info_block *ib;
	uint32_t status;

	if (info_block) {
		memset(info_block, 0, sizeof(*info_block));
	}

	if (vbe_detected == -1 || info_block) {
		memset(&controller_info, 0, sizeof(controller_info));
		vbe_detected = 0; /* mark as undetected */

		ib = (struct vbe_info_block *)SCRATCH_ADDR;
		memset(ib, 0, sizeof(*ib));

		ib->signature[0] = 'V';
		ib->signature[1] = 'B';
		ib->signature[2] = 'E';
		ib->signature[3] = '2';

		status = vbe_get_controller_info(ib);
		if (status == 0x4f) {
			memcpy(&controller_info, ib, sizeof(controller_info));
			vbe_detected = 1;
		}
	}

	if (!vbe_detected) {
		serial_print("VBE: VESA BIOS Extension not found!\n");
		return false;
	}

	if (info_block) {
		memcpy(info_block, &controller_info, sizeof(*info_block));
	}

	return true;
}

static int
bios_get_vbe_mode_info(struct vbe_mode_info_block *mode_info, uint32_t mode)
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


static int
vbe_get_mode_info(struct vbe_mode_info_block *mode_info, uint32_t mode)
{
	struct vbe_mode_info_block *mi_tmp = (struct vbe_mode_info_block *)SCRATCH_ADDR;

	if (!vbe_probe(0)) {
		return 0;	// the only real "error"
	}	

	/* if non-VESA, skip */
	if (mode < 0x100) {
		memset(mi_tmp, 0, sizeof(*mi_tmp));
		return -1;
	}

	if (bios_get_vbe_mode_info(mi_tmp, mode) != 0x004f) {
		return -2;	
	}

	memcpy(mode_info, mi_tmp, sizeof(*mode_info));
	return 1;	
}

static int
vbe_get_video_mode(uint32_t *mode)
{
	if (!vbe_probe(0)) {
		return -1;	
	}	
	
	struct int_regs regs = {0};
	regs.eax = 0x4f03;
	regs.flags = 0x200;
	
	rm_int(0x10, &regs);
	
	*mode = regs.ebx & 0xffff;
	return regs.eax & 0xffff;
}

static int
bios_set_video_mode(struct vbe_crtc_info_block *crtc_info, uint32_t mode)
{
	struct int_regs regs = {0};
	regs.es = (((uintptr_t)crtc_info) & 0xffff0000) >> 4;
  regs.edi = ((uintptr_t)crtc_info) & 0xffff;
  regs.eax = 0x4f02;
	regs.ebx = mode;
  regs.flags = 0x200;

  rm_int(0x10, &regs);

  return regs.eax & 0xffff;
}

static int
vbe_set_video_mode(struct vbe_mode_info_block *mode_info, uint32_t mode) 
{
	(void)mode_info;

	if (!vbe_probe(0)) {
		return -1;	
	}	

	// maybe try to get and if fail, err?

	/* For all VESA BIOS modes, force linear frame buffer.  */
	if (mode >= 0x100) {
		mode |= (1 << 14);
	} 

	if (bios_set_video_mode(0, mode) != 0x004f) {
		return -2;
	}

	return 0;
}

static void *
real2pm (uint32_t ptr)
{
	return (void *) ((((unsigned long) ptr & 0xFFFF0000) >> 12UL)
                   + ((unsigned long) ptr & 0x0000FFFF));
}

static bool
vbe_init(void)
{
	serial_print("VBE: Initializing...\n");
	struct vbe_info_block info_block;
	uint16_t *rm_vbe_mode_list, *p;
	size_t vbe_mode_list_size;
	
	if (!vbe_probe(&info_block)) {
		serial_print("VBE: ERROR: vbe_probe()\n");
		return false;
	}

	serial_print("VBE: Version: %d.%d\n", info_block.version_hi, info_block.version_lo);
	serial_print("VBE: OEM: %s\n", (char *)rm_desegment(info_block.oem_seg, info_block.oem_off));
	serial_print("VBE: Graphics vendor: %s\n", (char *)rm_desegment(info_block.vendor_seg, info_block.vendor_off));
	serial_print("VBE: Product name: %s\n", (char *)rm_desegment(info_block.prod_name_seg, info_block.prod_name_off));
	serial_print("VBE: Product revision: %s\n", (char *)rm_desegment(info_block.prod_rev_seg, info_block.prod_rev_off));

	uint32_t vm_ptr = rm_desegment(info_block.vid_modes_seg, info_block.vid_modes_off);
	p = rm_vbe_mode_list = real2pm(vm_ptr);
	while (*p++ != 0xffff);

	vbe_mode_list_size = (uintptr_t)p - (uintptr_t)rm_vbe_mode_list;
	vbe_mode_list = ext_mem_alloc(vbe_mode_list_size);

	memcpy(vbe_mode_list, rm_vbe_mode_list, vbe_mode_list_size);

	if (vbe_get_video_mode(&initial_vbe_mode) != 0x004f) {
		serial_print("VBE: ERROR: vbe_get_video_mode()\n");

		memmap_free(vbe_mode_list, vbe_mode_list_size);
		return false;
	}

	serial_print("VBE: Initialized\n");
	return true;
}

bool
vbe_setup(struct fb_info *ret, uint16_t target_width, uint16_t target_height, uint16_t target_bpp) 
{
	if (!vbe_init()) {
		serial_print("VBE: Error in vbe init\n");
		return false;
	}

	serial_print("VBE: Setting up...\n");

	struct vbe_mode_info_block vbe_mode_info;
	struct vbe_mode_info_block best_vbe_mode_info;
	uint32_t best_vbe_mode = 0;
	uint16_t *p;
	int preferred_mode = 0;

	if (!target_width || !target_height || !target_bpp) {
		struct edid_record *edid_record = get_edid_record();	
		if (edid_record != NULL) {
			int edid_width = edid_record->detailed_timings[0].horizontal_active_lo + ((edid_record->detailed_timings[0].horizontal_hi & 0xf0) << 4);
			int edid_height = edid_record->detailed_timings[0].vertical_active_lo + ((edid_record->detailed_timings[0].vertical_hi & 0xf0) << 4);

			if (edid_width && edid_height) {
				target_width = edid_width;
				target_height = edid_height;
				serial_print("VBE: EDID detected screen resolution of %dx%d\n", target_width, target_height);
				preferred_mode = 1;	
			}
		}
	}

	/* walkthrough mode list and try to find matching mode */
	for (p = vbe_mode_list; *p != 0xffff; ++p) {
		uint32_t vbe_mode = *p;

		int status = vbe_get_mode_info(&vbe_mode_info, vbe_mode);
		if (status == 0) {
			break;	
		}
		if (status < 0) {
			continue;
		}

		if ((vbe_mode_info.mode_attributes & VBE_MODEATTR_GRAPHICS) == 0) {
			continue;
		}
		if ((vbe_mode_info.mode_attributes & VBE_MODEATTR_LFB_AVAIL) == 0) {
			continue;
		}
		if ((vbe_mode_info.mode_attributes & VBE_MODEATTR_SUPPORTED) == 0) {
			continue;
		}
		if ((vbe_mode_info.mode_attributes & VBE_MODEATTR_COLOR) == 0) {
			continue;
		}
		if ((vbe_mode_info.memory_model != VBE_MEMORY_MODEL_PACKED_PIXEL) && (vbe_mode_info.memory_model != VBE_MEMORY_MODEL_DIRECT_COLOR)) {
			continue;
		}

		if (target_bpp != 0 && vbe_mode_info.bpp != target_bpp) {
			continue;
		}

		if (vbe_mode_info.bpp != 8
			&& vbe_mode_info.bpp != 15
			&& vbe_mode_info.bpp != 16
			&& vbe_mode_info.bpp != 24
			&& vbe_mode_info.bpp != 32) {
			continue;
		}

		if (preferred_mode) {
			if (vbe_mode_info.res_x > target_width || vbe_mode_info.res_y > target_height) {
				continue;
			}
		} else {
			if (((vbe_mode_info.res_x != target_width) || (vbe_mode_info.res_y != target_height))
					&& target_width != 0 && target_height != 0) {
				continue;
			}
		}

		/* select largest mode available (preferably target but we'll take what we can get) */
		if (best_vbe_mode != 0) {
			if ((uint64_t)vbe_mode_info.bpp * vbe_mode_info.res_x * vbe_mode_info.res_y < 
					(uint64_t)best_vbe_mode_info.bpp * best_vbe_mode_info.res_x * best_vbe_mode_info.res_y)	{
				continue;
			}
		}

		best_vbe_mode = vbe_mode;
		memcpy(&best_vbe_mode_info, &vbe_mode_info, sizeof(best_vbe_mode_info));
	}

	serial_print("VBE: Best vbe mode: 0x%x, ", best_vbe_mode);
	serial_print("%dx", best_vbe_mode_info.res_x);
	serial_print("%dx", best_vbe_mode_info.res_y);
	serial_print("%d\n", best_vbe_mode_info.bpp);

	/* try to set best VBE mode */
	if (best_vbe_mode != 0) {
		// DOESN'T WORK?
		if (vbe_set_video_mode(0, best_vbe_mode) != 0) {
			serial_print("VBE: ERROR: Could not set video mode 0x%x!\n", best_vbe_mode);
			return false;
		}
	} else {
		serial_print("VBE: ERROR: Could not find a matching video mode!\n");
		return false;
	}

	serial_print("VBE: Set the video mode for 0x%x\n", best_vbe_mode);

	ret->memory_model = best_vbe_mode_info.memory_model;
	ret->framebuffer_addr = best_vbe_mode_info.framebuffer_addr;
	ret->framebuffer_width = best_vbe_mode_info.res_x;
	ret->framebuffer_height = best_vbe_mode_info.res_y;
	ret->framebuffer_bpp = best_vbe_mode_info.bpp;

	if (controller_info.version_hi < 3) {
		ret->framebuffer_pitch = best_vbe_mode_info.bytes_per_scanline;
		ret->red_mask_size = best_vbe_mode_info.red_mask_size;
		ret->red_mask_shift = best_vbe_mode_info.red_mask_shift;
		ret->green_mask_size = best_vbe_mode_info.green_mask_size;
		ret->green_mask_shift = best_vbe_mode_info.green_mask_shift;
		ret->blue_mask_size = best_vbe_mode_info.blue_mask_size;
		ret->blue_mask_shift = best_vbe_mode_info.blue_mask_shift;
	} else {
		ret->framebuffer_pitch = best_vbe_mode_info.lin_bytes_per_scanline;
		ret->red_mask_size = best_vbe_mode_info.lin_red_mask_size;
		ret->red_mask_shift = best_vbe_mode_info.lin_red_mask_shift;
		ret->green_mask_size = best_vbe_mode_info.lin_green_mask_size;
		ret->green_mask_shift = best_vbe_mode_info.lin_green_mask_shift;
		ret->blue_mask_size = best_vbe_mode_info.lin_blue_mask_size;
		ret->blue_mask_shift = best_vbe_mode_info.lin_blue_mask_shift;
	}

	fb_clear(ret);

	return true;
}

