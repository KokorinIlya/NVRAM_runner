#ifndef DIPLOM_CONSTANTS_AND_TYPES_H
#define DIPLOM_CONSTANTS_AND_TYPES_H

#include <cstdint>

/**
 * All user functions must have this type.
 */
using function_ptr = void (*)(const uint8_t *);

/**
 * Size of persistent stack - 2 KB.
 */
extern const uint32_t PMEM_STACK_SIZE;

/**
 * Size of persistent heap - 2 MB.
 */
extern const uint64_t PMEM_HEAP_SIZE;

/**
 * Size of cache line on the current architecture - approximately 64 bytes.
 */
extern const uint32_t CACHE_LINE_SIZE;

/**
 * Size of page on the current architecture - approximately 4 KB.
 */
extern const uint32_t PAGE_SIZE;

#endif //DIPLOM_CONSTANTS_AND_TYPES_H
