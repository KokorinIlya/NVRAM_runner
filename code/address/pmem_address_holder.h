//
// Created by ilya on 28.04.2020.
//

#ifndef DIPLOM_PMEM_ADDRESS_HOLDER_H
#define DIPLOM_PMEM_ADDRESS_HOLDER_H

#include <stdint.h>

struct pmem_address_holder
{
    static uint8_t* pmem_addr;
};

#endif //DIPLOM_PMEM_ADDRESS_HOLDER_H
