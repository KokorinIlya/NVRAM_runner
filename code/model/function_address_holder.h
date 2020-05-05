//
// Created by ilya on 05.05.2020.
//

#ifndef DIPLOM_FUNCTION_ADDRESS_HOLDER_H
#define DIPLOM_FUNCTION_ADDRESS_HOLDER_H

#include <unordered_map>
#include "../common/constants_and_types.h"

struct function_address_holder
{
    std::unordered_map<std::string, std::pair<function_ptr, function_ptr>> funcs;

    function_address_holder();
};

#endif //DIPLOM_FUNCTION_ADDRESS_HOLDER_H
