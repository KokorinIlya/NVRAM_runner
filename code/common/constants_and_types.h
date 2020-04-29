//
// Created by ilya on 24.04.2020.
//

#ifndef DIPLOM_CONSTANTS_AND_TYPES_H
#define DIPLOM_CONSTANTS_AND_TYPES_H

#include <stdint.h>

using function_ptr = void (*)(const uint8_t *);

const uint32_t PMEM_STACK_SIZE = 2048; // 2 KB

const uint32_t CACHELINE_SIZE = 64; // 64 bytes

#endif //DIPLOM_CONSTANTS_AND_TYPES_H
