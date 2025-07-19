#ifndef __COMMON_ACPI_H__
#define __COMMON_ACPI_H__

#include <types.h>

/* reference: https://wiki.osdev.org/System_Management_BIOS */

struct smbios_entry_point_32
{
	char anchor_string[4];	
	uint8_t entry_point_checksum;
	uint8_t entry_point_length;
	uint8_t smbios_major_version;
	uint8_t smbios_minor_version;
	uint16_t smbios_max_structure_size;
	uint8_t entry_point_revision;
	char formatted_area[5];
	char intermediate_anchor_string[5];
	uint8_t intermediate_checksum;
	uint16_t structure_table_length;
	uint32_t structure_table_address;
	uint16_t num_structures;
	uint8_t bcd_revision;
} __attribute__((packed));

struct smbios_entry_point_64
{
	char anchor_string[5];	
	uint8_t entry_point_checksum;
	uint8_t entry_point_length;
	uint8_t smbios_major_version;
	uint8_t smbios_minor_version;
	uint8_t smbios_docrev;
	uint8_t entry_point_revision;
	uint8_t reserved;
	uint32_t structure_table_max_size; 
	uint64_t structure_table_address;
} __attribute__((packed));

/* reference: https://wiki.osdev.org/RSDT */
struct sdt 
{
	char     signature[4];
	uint32_t length;
	uint8_t  rev;
	uint8_t  checksum;
	char     oem_id[6];
	char     oem_table_id[8];
	uint32_t oem_rev;
	char     creator_id[4];
	uint32_t creator_rev;
} __attribute__((packed));

struct rsdp 
{
	char     signature[8];
	uint8_t  checksum;
	char     oem_id[6];
	uint8_t  rev;
	uint32_t rsdt_addr;
	// revision 2 only after this comment
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t  ext_checksum;
	uint8_t  reserved[3];
} __attribute__((packed));


struct rsdt 
{
	struct sdt header;
	char ptrs_start[];
} __attribute__((packed));


void acpi_get_smbios(void **smbios32, void **smbios64);

#endif // !__COMMON_ACPI_H__
