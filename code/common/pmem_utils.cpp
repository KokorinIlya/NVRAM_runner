#include "pmem_utils.h"
#include <libpmem.h>
#include <cassert>

// TODO: remove dependency from PMDK using msync(2)
void pmem_do_flush(const void* ptr, size_t len)
{
    pmem_msync(ptr, len);
}

uint64_t get_cache_line_aligned_address(uint64_t address)
{
    if (address % CACHE_LINE_SIZE == 0)
    {
        return address;
    }
    /*
     * 1111111111111111111111111111111111111111111111111111111111000000
     * '1' * 58 + '0' * 6
     */
    const uint64_t mask = 0xFFFFFFFFFFFFFFC0;
    assert(mask % CACHE_LINE_SIZE == 0);
    return address & mask + 64;
}
