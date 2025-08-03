#include <common/relocation.h>
#include <types.h>
#include <mm/pmm.h>

static inline __attribute__((always_inline)) bool
check_overlap(uint64_t base1, uint64_t top1, uint64_t base2, uint64_t top2)
{
	return (base1 >= base2 && base1 < top2) || (top1 > base2 && top1 <= top2);
}

bool
relocation_append(struct relocation_range *ranges, uint64_t *ranges_count, void *relocation, uint64_t *target, size_t length)
{
	if (*target == (uint64_t)-1) /* alloc on top */
	{
		uint64_t max_top = 0;	

		for (size_t i = 0; i < *ranges_count; ++i)
		{
			uint64_t range_top = ranges[i].target + ranges[i].length;

			if (range_top > max_top)
			{
				max_top = range_top;
			}
		}
		*target = ALIGN_UP(max_top, PAGE_SIZE);
	}

	uint64_t max_retries = 0x10000;

retry:
	if (max_retries-- == 0)
	{
		return false;
	}

	for (size_t i = 0; i < *ranges_count; ++i)
	{
		uint64_t top = *target + length;

		/* check overlap with relocation range targets */
		{
			uint64_t range_base = ranges[i].target;
			uint64_t range_length = ranges[i].length;
			uint64_t range_top = range_base + range_length;

			if (check_overlap(range_base, range_top, *target, top))
			{
				*target = ALIGN_UP(top, PAGE_SIZE);
				goto retry;
			}
		}

		/* check overlap with relocation range sources */
		{
			uint64_t range_base = ranges[i].relocation;
			uint64_t range_length = ranges[i].length;
			uint64_t range_top = range_base + range_length;

			if (check_overlap(range_base, range_top, *target, top))
			{
				*target += PAGE_SIZE;
				goto retry;
			}
		}

		/* assert memory exists */
		if (!memmap_alloc_range(*target, length, MEMMAP_BOOTLOADER_RECLAIMABLE, MEMMAP_USABLE, false, false))
		{
			if (!memmap_alloc_range(*target, length, MEMMAP_BOOTLOADER_RECLAIMABLE, MEMMAP_BOOTLOADER_RECLAIMABLE, false, false))
			{
				*target += PAGE_SIZE;	
				goto retry;
			}
		}
	}

	ranges[*ranges_count].relocation = (uintptr_t)relocation;
	ranges[*ranges_count].target = *target;
	ranges[*ranges_count].length = length;
	*ranges_count += 1;
	
	return true;
}
