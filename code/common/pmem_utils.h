#ifndef DIPLOM_PMEM_UTILS_H
#define DIPLOM_PMEM_UTILS_H

#include <cstdint>
#include <cstddef>
#include "constants_and_types.h"

/**
 * Forces all memory in the range [addr, addr+len) to be stored durably
 * in persistent memory. All pointers from the range should point to
 * persistent memory. If the range fits into single cache line, flush is done
 * atomically. Otherwise, crash can occur when part of data has already been
 * flushed, but the rest of the range has not been flushed yet. Note, that on
 * non_NVRAM systems flush of more than one byte can be non-atomic operation.
 * @param ptr - beginning of the range.
 * @param len - length of the range.
 */
void pmem_do_flush(const void* ptr, size_t len);

/**
 * Returns minimal possible x, such that x >= addr and x % CACHE_LINE_SIZE == 0.
 * @param address - address to start searching aligned address from.
 * @return aligned address.
 */
uint64_t get_cache_line_aligned_address(uint64_t address);

/**
 * Returns true if address is part of caller threads's persistent stack and false otherwise.
 * If caller thread doesn't have persistent stack
 * (i.e. thread_local_non_owning_storage<persistent_stack> hasn't been set) std::runtime_error
 * is thrown.
 * @param address - address.
 * @return true if address is part of caller threads's persistent stack and false otherwise
 * @throws std::runtime_error if caller thread doesn't have persistent stack.
 */
bool is_stack_address(const uint8_t* address);

#endif //DIPLOM_PMEM_UTILS_H
