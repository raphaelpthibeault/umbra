#ifndef __MM_MEMORY_H__
#define __MM_MEMORY_H__

#include <types.h>

/* it would be really nice if C had C++'s enum : T syntax */

/* Page Bits (number of bits for page size e.g. 4K = 2**12) */
#define MEMORY_PGB_INV      0
#define MEMORY_PGB_1K       10
#define MEMORY_PGB_2K       11
#define MEMORY_PGB_4K       12
#define MEMORY_PGB_8K       13
#define MEMORY_PGB_16K      14
#define MEMORY_PGB_32K      15
#define MEMORY_PGB_64K      16
#define MEMORY_PGB_128K     17
#define MEMORY_PGB_256K     18
#define MEMORY_PGB_512K     19
#define MEMORY_PGB_1M       20
#define MEMORY_PGB_2M       21
#define MEMORY_PGB_4M       22
#define MEMORY_PGB_8M       23
#define MEMORY_PGB_16M      24
#define MEMORY_PGB_32M      25
#define MEMORY_PGB_64M      26
#define MEMORY_PGB_128M     27
#define MEMORY_PGB_256M     28
#define MEMORY_PGB_512M     29
#define MEMORY_PGB_1G       30
#define MEMORY_PGB_2G       31
#define MEMORY_PGB_4G       32
#define MEMORY_PGB_8G       33
#define MEMORY_PGB_16G      34
#define MEMORY_PGB_32G      35
#define MEMORY_PGB_64G      36
#define MEMORY_PGB_128G     37
#define MEMORY_PGB_256G     38
#define MEMORY_PGB_512G     39

/* Page Size (in bytes) */
#define MEMORY_PGS_1K       (1ULL << MEMORY_PGB_1K)
#define MEMORY_PGS_2K       (1ULL << MEMORY_PGB_2K)
#define MEMORY_PGS_4K       (1ULL << MEMORY_PGB_4K)
#define MEMORY_PGS_8K       (1ULL << MEMORY_PGB_8K)
#define MEMORY_PGS_16K      (1ULL << MEMORY_PGB_16K)
#define MEMORY_PGS_32K      (1ULL << MEMORY_PGB_32K)
#define MEMORY_PGS_64K      (1ULL << MEMORY_PGB_64K)
#define MEMORY_PGS_128K     (1ULL << MEMORY_PGB_128K)
#define MEMORY_PGS_256K     (1ULL << MEMORY_PGB_256K)
#define MEMORY_PGS_512K     (1ULL << MEMORY_PGB_512K)
#define MEMORY_PGS_1M       (1ULL << MEMORY_PGB_1M)
#define MEMORY_PGS_2M       (1ULL << MEMORY_PGB_2M)
#define MEMORY_PGS_4M       (1ULL << MEMORY_PGB_4M)
#define MEMORY_PGS_8M       (1ULL << MEMORY_PGB_8M)
#define MEMORY_PGS_16M      (1ULL << MEMORY_PGB_16M)
#define MEMORY_PGS_32M      (1ULL << MEMORY_PGB_32M)
#define MEMORY_PGS_64M      (1ULL << MEMORY_PGB_64M)
#define MEMORY_PGS_128M     (1ULL << MEMORY_PGB_128M)
#define MEMORY_PGS_256M     (1ULL << MEMORY_PGB_256M)
#define MEMORY_PGS_512M     (1ULL << MEMORY_PGB_512M)
#define MEMORY_PGS_1G       (1ULL << MEMORY_PGB_1G)
#define MEMORY_PGS_2G       (1ULL << MEMORY_PGB_2G)
#define MEMORY_PGS_4G       (1ULL << MEMORY_PGB_4G)
#define MEMORY_PGS_8G       (1ULL << MEMORY_PGB_8G)
#define MEMORY_PGS_16G      (1ULL << MEMORY_PGB_16G)
#define MEMORY_PGS_32G      (1ULL << MEMORY_PGB_32G)
#define MEMORY_PGS_64G      (1ULL << MEMORY_PGB_64G)
#define MEMORY_PGS_128G     (1ULL << MEMORY_PGB_128G)
#define MEMORY_PGS_256G     (1ULL << MEMORY_PGB_256G)
#define MEMORY_PGS_512G     (1ULL << MEMORY_PGB_512G)

/* Page Masks */
#define MEMORY_PGM_1K       (MEMORY_PGS_1K - 1)
#define MEMORY_PGM_2K       (MEMORY_PGS_2K - 1)
#define MEMORY_PGM_4K       (MEMORY_PGS_4K - 1)
#define MEMORY_PGM_8K       (MEMORY_PGS_8K - 1)
#define MEMORY_PGM_16K      (MEMORY_PGS_16K - 1)
#define MEMORY_PGM_32K      (MEMORY_PGS_32K - 1)
#define MEMORY_PGM_64K      (MEMORY_PGS_64K - 1)
#define MEMORY_PGM_128K     (MEMORY_PGS_128K - 1)
#define MEMORY_PGM_256K     (MEMORY_PGS_256K - 1)
#define MEMORY_PGM_512K     (MEMORY_PGS_512K - 1)
#define MEMORY_PGM_1M       (MEMORY_PGS_1M - 1)
#define MEMORY_PGM_2M       (MEMORY_PGS_2M - 1)
#define MEMORY_PGM_4M       (MEMORY_PGS_4M - 1)
#define MEMORY_PGM_8M       (MEMORY_PGS_8M - 1)
#define MEMORY_PGM_16M      (MEMORY_PGS_16M - 1)
#define MEMORY_PGM_32M      (MEMORY_PGS_32M - 1)
#define MEMORY_PGM_64M      (MEMORY_PGS_64M - 1)
#define MEMORY_PGM_128M     (MEMORY_PGS_128M - 1)
#define MEMORY_PGM_256M     (MEMORY_PGS_256M - 1)
#define MEMORY_PGM_512M     (MEMORY_PGS_512M - 1)
#define MEMORY_PGM_1G       (MEMORY_PGS_1G - 1)
#define MEMORY_PGM_2G       (MEMORY_PGS_2G - 1)
#define MEMORY_PGM_4G       (MEMORY_PGS_4G - 1)
#define MEMORY_PGM_8G       (MEMORY_PGS_8G - 1)
#define MEMORY_PGM_16G      (MEMORY_PGS_16G - 1)
#define MEMORY_PGM_32G      (MEMORY_PGS_32G - 1)
#define MEMORY_PGM_64G      (MEMORY_PGS_64G - 1)
#define MEMORY_PGM_128G     (MEMORY_PGS_128G - 1)
#define MEMORY_PGM_256G     (MEMORY_PGS_256G - 1)
#define MEMORY_PGM_512G     (MEMORY_PGS_512G - 1)

#endif // !__MM_MEMORY_H__
