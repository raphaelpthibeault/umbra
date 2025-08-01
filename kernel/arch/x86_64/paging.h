#include <types.h>
#include <mm/memory.h>

#define PAGING_PAGE_SIZE MEMORY_PGS_4K
#define PAGING_PAGE_BITS MEMORY_PGB_4K
#define PAGING_PAGE_MASK MEMORY_PGM_4K

#define PAGE_VALID_SIZES MEMORY_PGS_4K | MEMORY_PGS_2M | MEMORY_PGS_1G

#define	MIN_PAGE_BITS MEMORY_PGB_4K
#define	MAX_PAGE_BITS MEMORY_PGB_1G

#define MIN_PAGE_SIZE MEMORY_PGS_4K
#define MAX_PAGE_SIZE MEMORY_PGS_1G

#define MIN_PAGE_MASK MEMORY_PGM_4K
#define MAX_PAGE_MASK MEMORY_PGM_1G


// ------------------------------------------------

#define PAGING_FLAG_PRESENT         0x001
#define PAGING_FLAG_WRITE           0x002
#define PAGING_FLAG_USER            0x004
#define PAGING_FLAG_WRITE_THROUGH   0x008
#define PAGING_FLAG_CACHE_DISABLE   0x010
#define PAGING_FLAG_ACCESSED        0x020
#define PAGING_FLAG_LARGER_PAGES    0x040
#define PAGING_FLAG_OS_AVAILABLE    0xE00
#define PAGING_FLAG_NO_EXECUTE      (1 << 63)

#define PAGING_FLAGS_KERNEL_PAGE    (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE)
#define PAGING_FLAGS_USER_PAGE      (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_USER)
#define PAGING_FLAGS_MMIO_PAGE      (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_CACHE_DISABLE | PAGING_FLAG_WRITE_THROUGH)

// ------------------------------------------------

#define PAGING_KERNEL_OFFSET        0xffffffff80000000
#define PAGING_VIRTUAL_OFFSET       0xffff800000000000

#define nullvptr                    - PAGING_VIRTUAL_OFFSET == NULL

typedef uint64_t paging_desc_t;

typedef struct {
	uint16_t pml4;
	uint16_t pml3;
	uint16_t pml2;
	uint16_t pml1;
} __attribute__((packed)) paging_indexer_t;

typedef struct { 
	paging_desc_t entries[512];
} __attribute__((aligned(PAGING_PAGE_SIZE))) paging_table_t;

static __attribute__((always_inline)) inline void * 
paging_desc_get_address(paging_desc_t* descriptor) 
{
	return (void*)(*descriptor & 0xffffffffff000);
}

static __attribute__((always_inline)) inline void 
paging_desc_set_address(paging_desc_t* descriptor, uint64_t address) 
{
	*descriptor |= (uint64_t)address & 0xffffffffff000;
}

static __attribute__((always_inline)) inline bool 
paging_desc_get_flag(paging_desc_t* descriptor, uint64_t flag) 
{
	return *descriptor & flag;
}

static __attribute__((always_inline)) inline void 
paging_desc_set_flag(paging_desc_t* descriptor, uint64_t flag, bool value) 
{
	*descriptor &= ~flag;
	if (value)
	{
		*descriptor |= flag;
	}
}

static __attribute__((always_inline)) inline void 
paging_desc_set_flags(paging_desc_t* descriptor, uint64_t flags) 
{
	*descriptor &= ~0x0ffful;
	*descriptor |= flags;
}

static __attribute__((always_inline)) inline void
paging_indexer_assign(paging_indexer_t* indexer, void* address) 
{
	uint64_t uaddress = (uint64_t)address;
	uaddress >>= 12;
	indexer->pml1 = uaddress & 0x1ff;
	uaddress >>= 9;
	indexer->pml2 = uaddress & 0x1ff;
	uaddress >>= 9;
	indexer->pml3 = uaddress & 0x1ff;
	uaddress >>= 9;
	indexer->pml4 = uaddress & 0x1ff;
}


void *paging_map_page(void* virt, void* phys, uint16_t flags);


