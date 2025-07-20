#ifndef __LIB_ELF_H__
#define __LIB_ELF_H__

#include <types.h>
#include <common/relocation.h>

#define EI_MAG0		0			/* File identification byte 0 index */
#define ELFMAG0		0x7f	/* Magic number byte 0 */
#define EI_MAG1		1			/* File identification byte 1 index */
#define ELFMAG1		'E'		/* Magic number byte 1 */
#define EI_MAG2		2			/* File identification byte 2 index */
#define ELFMAG2		'L'		/* Magic number byte 2 */
#define EI_MAG3		3			/* File identification byte 3 index */
#define ELFMAG3		'F'		/* Magic number byte 3 */

#define EI_NIDENT (16)

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

struct elf64_phdr
{
  uint32_t	type;			/* Segment type */
  uint32_t	flags;		/* Segment flags */
  uint64_t	offset;		/* Segment file offset */
  uint64_t	vaddr;		/* Segment virtual address */
  uint64_t	paddr;		/* Segment physical address */
  uint64_t	filesz;		/* Segment size in file */
  uint64_t	memsz;		/* Segment size in memory */
  uint64_t	align;		/* Segment alignment */
};

struct elf64_shdr
{
  uint32_t	name;		/* Section name (string tbl index) */
  uint32_t	type;		/* Section type */
  uint64_t	flags;	/* Section flags */
  uint64_t	addr;		/* Section virtual addr at execution */
  uint64_t	offset;	/* Section file offset */
  uint64_t	size;		/* Section size in bytes */
  uint32_t	link;		/* Link to another section */
  uint32_t	info;		/* Additional section information */
  uint64_t	addralign;	/* Section alignment */
  uint64_t	entsize;		/* Entry size if section holds table */
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
	uint32_t num;
	uint32_t section_entry_size;
	uint32_t str_section_idx;
	uint32_t section_offset;
};

struct elf_shdr_info elf64_get_elf_shdr_info(uint8_t *elf);
bool elf64_load_relocation(uint8_t *elf, uint64_t *entry_point, struct relocation_range **ranges);

#endif // !__LIB_ELF_H__
