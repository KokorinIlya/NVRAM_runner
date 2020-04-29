//
// Created by ilya on 29.04.2020.
//

#ifndef DIPLOM_PMEM_UTILS_H
#define DIPLOM_PMEM_UTILS_H

#include <cstdint>
#include <cstddef>

void pmem_do_flush(const void* ptr, size_t len);

#endif //DIPLOM_PMEM_UTILS_H
