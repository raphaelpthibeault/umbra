#ifndef __LIB_ELF_H__
#define __LIB_ELF_H__

#include <types.h>

#define EI_MAG0		0			/* File identification byte 0 index */
#define ELFMAG0		0x7f	/* Magic number byte 0 */
#define EI_MAG1		1			/* File identification byte 1 index */
#define ELFMAG1		'E'		/* Magic number byte 1 */
#define EI_MAG2		2			/* File identification byte 2 index */
#define ELFMAG2		'L'		/* Magic number byte 2 */
#define EI_MAG3		3			/* File identification byte 3 index */
#define ELFMAG3		'F'		/* Magic number byte 3 */

#define EI_NIDENT (16)

struct elf32_ehdr
{
  uint8_t		ident[EI_NIDENT];	/* Magic number and other info */
  uint16_t	type;				/* Object file type */
  uint16_t	machine;		/* Architecture */
  uint32_t	version;		/* Object file version */
  uint32_t	entry;			/* Entry point virtual address */
  uint32_t	phoff;			/* Program header table file offset */
  uint32_t	shoff;			/* Section header table file offset */
  uint32_t	flags;			/* Processor-specific flags */
  uint16_t	ehsize;			/* ELF header size in bytes */
  uint16_t	phentsize;	/* Program header table entry size */
  uint16_t	phnum;			/* Program header table entry count */
  uint16_t	shentsize;	/* Section header table entry size */
  uint16_t	shnum;			/* Section header table entry count */
  uint16_t	shstrndx;		/* Section header string table index */
};

struct elf64_ehdr
{
  uint8_t		ident[EI_NIDENT];	/* Magic number and other info */
  uint16_t	type;				/* Object file type */
  uint16_t	machine;		/* Architecture */
  uint32_t	version;		/* Object file version */
  uint64_t	entry;			/* Entry point virtual address */
  uint64_t	phoff;			/* Program header table file offset */
  uint64_t	shoff;			/* Section header table file offset */
  uint32_t	flags;			/* Processor-specific flags */
  uint16_t	ehsize;			/* ELF header size in bytes */
  uint16_t	phentsize;	/* Program header table entry size */
  uint16_t	phnum;			/* Program header table entry count */
  uint16_t	shentsize;	/* Section header table entry size */
  uint16_t	shnum;			/* Section header table entry count */
  uint16_t	shstrndx;		/* Section header string table index */
};

struct elf64_hdr
{
	uint8_t  ident[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t hdr_size;
	uint16_t phdr_size;
	uint16_t ph_num;
	uint16_t shdr_size;
	uint16_t sh_num;
	uint16_t shstrndx;
};

struct elf64_shdr 
{
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
};

struct elf32_shdr 
{
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
};

struct elf64_sym 
{
	uint32_t st_name;
	uint8_t  st_info;
	uint8_t  st_other;
	uint16_t st_shndx;
	uint64_t st_value;
	uint64_t st_size;
};

struct elf_shdr_info
{
	uint32_t section_entry_size;
	uint32_t str_section_idx;
	uint32_t num;
	uint32_t section_offset;
};

int elf_bits(uint8_t *elf);

#endif // !__LIB_ELF_H__
