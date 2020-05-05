//
// Created by ilya on 05.05.2020.
//

#ifndef DIPLOM_FUNCTION_ADDRESS_HOLDER_H
#define DIPLOM_FUNCTION_ADDRESS_HOLDER_H

#include <unordered_map>
#include "../common/constants_and_types.h"

/**
 * Stores mapping from function name to pair of pointers - pointer to function itself and
 * pointer to recovery version of the function. All functions, that will be used in program,
 * should be registered in this mapping before starting execution or restoration.
 * Since there should be only only instance of such mapping, it is proposed to use
 * this class with global_storage<T>.
 */
struct function_address_holder
{
    std::unordered_map<std::string, std::pair<function_ptr, function_ptr>> funcs;

    function_address_holder();
};

#endif //DIPLOM_FUNCTION_ADDRESS_HOLDER_H
