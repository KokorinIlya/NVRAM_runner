#include "pmem_utils.h"
#include <libpmem.h>
#include <cassert>
#include "../storage/thread_local_non_owning_storage.h"
#include "../persistent_stack/persistent_stack.h"

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
    /* For CACHE_LINE_SIZE == 64:
     *  mask == 0xFFFFFFFFFFFFFFC0
     *  mask == 0b1111111111111111111111111111111111111111111111111111111111000000
     *  mask == '1' * 58 + '0' * 6
     */
    const uint64_t mask = ~(CACHE_LINE_SIZE - 1);
    assert(mask % CACHE_LINE_SIZE == 0);
    uint64_t result = (address & mask) + CACHE_LINE_SIZE;
    assert(result % CACHE_LINE_SIZE == 0);
    return result;
}

bool is_stack_address(const uint8_t* address)
{
    const uint8_t* stack_begin_address =
            thread_local_non_owning_storage<persistent_stack>::ptr->get_stack_ptr();
    return address >= stack_begin_address && address < stack_begin_address + PMEM_STACK_SIZE;
}
