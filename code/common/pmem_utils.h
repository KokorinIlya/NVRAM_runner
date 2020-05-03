//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_PMEM_UTILS_H
#define DIPLOM_PMEM_UTILS_H

#include <cstdint>
#include <cstddef>

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

#endif //DIPLOM_PMEM_UTILS_H
