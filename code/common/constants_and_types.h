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
 * Size of cache line on the current architecture - approximately 64 bytes.
 */
const uint32_t CACHE_LINE_SIZE = sysconf (_SC_LEVEL1_DCACHE_LINESIZE);

/**
 * Size of page on the current architecture - approximately 4 KB.
 */
const uint32_t PAGE_SIZE = getpagesize();

#endif //DIPLOM_CONSTANTS_AND_TYPES_H
