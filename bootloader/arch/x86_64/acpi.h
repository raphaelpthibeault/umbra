#ifndef __ACPI_H__
#define __ACPI_H__

#include <types.h>
#include "cpu.h"

#define EBDA (ebda_get())

static inline uintptr_t
ebda_get(void) 
{
	uintptr_t ebda = (uintptr_t)mminw(0x40e) << 4;

	/* perform sanity checks 
	 * ebda memory is supposed to be (inclusive): 0x80000 - 0xa0000
	 * */
	if (ebda < 0x80000 || ebda >= 0xa0000) {
		ebda = 0x80000;
	}

	return ebda;
}

#endif // !__ACPI_H__
