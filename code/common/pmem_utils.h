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
 * flushed, but the rest of the range has not been flushed yet.
 * @param ptr - beginning of the range.
 * @param len - length of the range.
 */
void pmem_do_flush(const void* ptr, size_t len);

// TODO: test
/**
 * Returns minimal possible x, such that x >= addr and x % CACHE_LINE_SIZE == 0.
 * @param address - address to start searching aligned address from.
 * @return aligned address.
 */
uint64_t get_cache_line_aligned_address(uint64_t address);

#endif //DIPLOM_PMEM_UTILS_H
