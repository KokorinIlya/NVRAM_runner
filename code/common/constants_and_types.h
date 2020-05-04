#ifndef DIPLOM_CONSTANTS_AND_TYPES_H
#define DIPLOM_CONSTANTS_AND_TYPES_H

#include <stdint.h>
#include <unistd.h>

/**
 * All user functions must have this type.
 */
using function_ptr = void (*)(const uint8_t *);

/**
 * Size of persistent stack - 2 KB.
 */
const uint32_t PMEM_STACK_SIZE = 2048;

/**
 * Size of cache line on the current architecture - 64 bytes.
 */
const uint32_t CACHE_LINE_SIZE = 64;

/**
 * Size of page on the current architecture - 4 KB
 */
const uint32_t PAGE_SIZE = getpagesize();

#endif //DIPLOM_CONSTANTS_AND_TYPES_H
