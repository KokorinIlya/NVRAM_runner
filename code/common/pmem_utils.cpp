#include "pmem_utils.h"
#include <libpmem.h>
#include <cassert>
#include "../storage/thread_local_non_owning_storage.h"
#include "../storage/global_non_owning_storage.h"
#include "../persistent_memory/persistent_memory_holder.h"

// TODO: remove dependency from PMDK using msync(2)
void pmem_do_flush(const void* ptr, size_t len)
{
#ifdef REAL_NVRAM
    pmem_persist(ptr, len);
#else
    pmem_msync(ptr, len);
#endif
}

uint64_t get_cache_line_aligned_address(uint64_t address)
{
    /*
     * Cache line size must be power of two.
     */
    assert((CACHE_LINE_SIZE & (CACHE_LINE_SIZE - 1)) == 0);

    if (address % CACHE_LINE_SIZE == 0)
    {
        return address;
    }
    /* For CACHE_LINE_SIZE == 64:
     *  mask == 0xFFFFFFFFFFFFFFC0
     *  mask == 0b1111111111111111111111111111111111111111111111111111111111000000
     *  mask == '1' * 58 + '0' * 6
     */
    const uint64_t mask = ~(CACHE_LINE_SIZE - 1);
    assert(mask % CACHE_LINE_SIZE == 0);
    const uint64_t result = (address & mask) + CACHE_LINE_SIZE;
    assert(result % CACHE_LINE_SIZE == 0);
    return result;
}

bool is_valid_address(const uint8_t* block_begin_address, uint64_t block_size, const uint8_t* address)
{
    return address >= block_begin_address && address < block_begin_address + block_size;
}

bool is_stack_address(const uint8_t* address)
{
    const uint8_t* const stack_begin_address =
            thread_local_non_owning_storage<persistent_memory_holder>::ptr->get_pmem_ptr();
    return is_valid_address(stack_begin_address, PMEM_STACK_SIZE, address);
}

bool is_heap_address(const uint8_t* address)
{
    const uint8_t* const heap_begin_address = global_non_owning_storage<persistent_memory_holder>::ptr->get_pmem_ptr();
    return is_valid_address(heap_begin_address, PMEM_HEAP_SIZE, address);
}
