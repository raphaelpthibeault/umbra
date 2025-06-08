#include "edid.h"
#include <types.h>
#include <mm/pmm.h>
#include <arch/x86_64/real.h>
#include <drivers/serial.h>
#include <lib/misc.h>

struct edid_record *
get_edid_record(void)
{
	/* static, there's only one monitor after all */
	static struct edid_record *edid_record = NULL;

	if (edid_record == NULL) {
		edid_record = ext_mem_alloc(sizeof(struct edid_record));
	}

	static struct edid_record *buf = (struct edid_record *)SCRATCH_ADDR;
	memset(buf, 0, sizeof(struct edid_record));

	/* BIOS - INT 0x10, AX=4F15h, BL=01h, CX = Controller Unit, DX = EDID block, ES:DI = 128-Byte Buffer.  */	

	struct int_regs regs = {0};
	regs.eax = 0x4f15;
  regs.ebx = 0x0001;
  regs.ecx = 0x0000;
  regs.edx = 0x0000;
	regs.es = (((uintptr_t)buf) & 0xffff0000) >> 4;
  regs.edi = ((uintptr_t)buf) & 0xffff;
	regs.flags = 0x200;

	rm_int(0x10, &regs);
	
	if (regs.eax != 0x004F) {
		goto fail;
	}

	for (size_t i = 0; i < sizeof(struct edid_record); ++i) {
		if (((uint8_t *)buf)[i] != 0) {
			goto success;
		}
	}

	serial_print("[error] zeroed edid recrod buffer\n");

fail:
	serial_print("[error] Could not get edid record\n");
	return NULL;
success:
	memcpy(edid_record, buf, sizeof(struct edid_record));
	serial_print("EDID: got edid record\n");
	return edid_record;
}

