#include <types.h>
#include "vga.h"
#include <misc.h>

static const char *exception_names[] = {
    "Division",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "???",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "???",
    "x87",
    "Alignment check",
    "Machine check",
    "SIMD",
    "Virtualisation",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "Security"
};

void 
except(uint32_t exception, uint32_t error_code, uint32_t ebp, uint32_t eip)
{
	(void)ebp;
	putstr("[Panic] ", COLOR_RED, COLOR_BLK);
	putstr(exception_names[exception], COLOR_RED, COLOR_BLK);
	putstr(" exception at 0x", COLOR_RED, COLOR_BLK);
	{
		char res[32];
		itoa(eip, res, 16);
		putstr(res, COLOR_RED, COLOR_BLK); 
	}
	putstr(" code: ", COLOR_RED, COLOR_BLK);
	{
		char res[32];
		itoa(error_code, res, 16);
		putstr(res, COLOR_RED, COLOR_BLK); 
	}
	putstr("\n", COLOR_RED, COLOR_BLK);
}

