#include <common/acpi.h>
#include <types.h>
#include <lib/misc.h>
#include <drivers/serial.h>

/* reference: https://github.com/managarm/lai/blob/master/helpers/pc-bios.c */
static uint8_t
acpi_calculate_checksum(void *ptr, size_t size)
{
	uint8_t sum = 0, *_ptr = ptr;

	for (size_t i = 0; i < size; ++i)
	{
		sum += _ptr[i];
	}

	return sum;
}

/* reference: https://wiki.osdev.org/System_Management_BIOS */
void 
acpi_get_smbios(void **smbios32, void **smbios64)
{
	*smbios32 = NULL;
	*smbios64 = NULL;

	for (size_t i = 0xf0000; i < 0x100000; i += 16) // 16-byte aligned addresses
	{
		struct smbios_entry_point_32 *eps = (struct smbios_entry_point_32 *)i;
		
		if (memcmp(eps->anchor_string, "_SM_", 4) == 0
				&& acpi_calculate_checksum(eps, eps->entry_point_length) == 0)
		{
			serial_print("ACPI: Found 32-bit smbios entry-point at 0x%x\n", eps);	
			*smbios32 = (void *)eps;
			break;
		}
	}

	for (size_t i = 0xf0000; i < 0x100000; i += 16) 
	{
		struct smbios_entry_point_64 *eps = (struct smbios_entry_point_64 *)i;
		
		if (memcmp(eps->anchor_string, "_SM3_", 5) == 0
				&& acpi_calculate_checksum(eps, eps->entry_point_length) == 0)
		{
			serial_print("ACPI: Found 64-bit smbios entry-point at 0x%x\n", eps);	
			*smbios64 = (void *)eps;
			break;
		}
	}
}
