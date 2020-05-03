//
// Created by ilya on 24.04.2020.
//

#ifndef DIPLOM_CONSTANTS_AND_TYPES_H
#define DIPLOM_CONSTANTS_AND_TYPES_H

#include <stdint.h>

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
const uint32_t CACHELINE_SIZE = 64;

#endif //DIPLOM_CONSTANTS_AND_TYPES_H
