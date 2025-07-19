#include <lib/elf.h>
#include <lib/misc.h>
#include <types.h>
#include <drivers/serial.h>

#define ARCH_X86_64  0x3e
#define ARCH_X86_32  0x03

int
elf_bits(uint8_t *elf)
{
	struct elf64_hdr *hdr = (void *)elf;
	
	if (strncmp((char *)hdr->ident, "\177ELF", 4) != 0)
	{
		return -1;
	}

	switch (hdr->machine)
	{
		case ARCH_X86_64:
			return 64;
		case ARCH_X86_32:
			return 32;
		default:
			return -1;
	}
}

